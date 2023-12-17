#include "pch.h"

#include <Engine/Graphics/GraphicsDevice.h>

#include <filesystem>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx12.h>

#include <Engine/Graphics/Shader.h>

// ReSharper disable CppNonInlineVariableDefinitionInHeaderFile
extern "C" {
// http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;

//  https://gpuopen.com/amdpowerxpressrequesthighperformance/
_declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001;
}

// ReSharper enable CppNonInlineVariableDefinitionInHeaderFile

FGraphicsDevice::FGraphicsDevice(const FGraphicsDeviceSettings& InSettings)
	: Settings(InSettings), BufferCount(GetBufferingModeCount(Settings.BufferingMode))
{
	SB_LOG_INFO("Renderer Buffering Mode: {}", GetBufferingModeName(Settings.BufferingMode));
	if (Settings.bEnableDebugLayer)
	{
		const HRESULT EnableDebugLayerResult = D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController));
		SB_D3D_ASSERT(EnableDebugLayerResult, "Failed to enable debug layer");
		DebugController->EnableDebugLayer();
		DebugController->SetEnableGPUBasedValidation(Settings.bEnableGPUValidation);

		const HRESULT EnableDXGIDebugResult = DXGIGetDebugInterface1(0, IID_PPV_ARGS(&DXGIDebugController));
		SB_D3D_ASSERT(EnableDXGIDebugResult, "Failed to enable DXGI debug");
		DXGIDebugController->EnableLeakTrackingForThread();
		SB_LOG_INFO("D3D12 debug layer enabled");
	}
	const uint32_t FactoryCreationFlags = Settings.bEnableDebugLayer ? DXGI_CREATE_FACTORY_DEBUG : 0;
	const HRESULT CreateFactoryResult = CreateDXGIFactory2(FactoryCreationFlags, IID_PPV_ARGS(&Factory));
	SB_D3D_ASSERT(CreateFactoryResult, "Failed to create DXGI factory");
	ComPointer<IDXGIAdapter1> Adapter;
	DXGI_ADAPTER_DESC1 AdapterDesc;
	uint32_t AdapterIndex = 0;
	bool bAdapterFound = false;
	while (Factory->EnumAdapterByGpuPreference(AdapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
	                                           IID_PPV_ARGS(&Adapter)) != DXGI_ERROR_NOT_FOUND)
	{
		Adapter->GetDesc1(&AdapterDesc);
		if (AdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			AdapterIndex++;
			continue;
		}
		const HRESULT DeviceCreateResult = D3D12CreateDevice(Adapter, Settings.FeatureLevel, _uuidof(ID3D12Device),
		                                                     nullptr);
		if (SUCCEEDED(DeviceCreateResult))
		{
			bAdapterFound = true;
			break;
		}
		Adapter.Release();
		AdapterIndex++;
	}
	SB_ASSERT_CRITICAL(bAdapterFound, E_FAIL, "Failed to find suitable adapter");
	char AdapterNameBuffer[128];
	WideCharToMultiByte(CP_ACP, 0, AdapterDesc.Description, -1, AdapterNameBuffer, 128, nullptr, nullptr);
	std::string AdapterName(AdapterNameBuffer);
	SB_LOG_INFO("Using '{}' adapter", AdapterName);
	SB_LOG_INFO("\t- Dedicated video memory: {} MB", AdapterDesc.DedicatedVideoMemory / 1024 / 1024);
	SB_LOG_INFO("\t- Shared system memory: {} MB", AdapterDesc.SharedSystemMemory / 1024 / 1024);

	const HRESULT DeviceCreateResult = D3D12CreateDevice(Adapter, Settings.FeatureLevel, IID_PPV_ARGS(&Device));
	SB_D3D_ASSERT(DeviceCreateResult, "Failed to create D3D12 device");
	Adapter.Release();

	D3D12_COMMAND_QUEUE_DESC CommandQueueDesc;
	CommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	CommandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
	CommandQueueDesc.NodeMask = 0;
	CommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	const HRESULT CommandQueueCreateResult = Device->CreateCommandQueue(&CommandQueueDesc, IID_PPV_ARGS(&CommandQueue));
	SB_D3D_ASSERT(CommandQueueCreateResult, "Failed to create command queue");

	CommandLists.reserve(BufferCount);
	Fences.reserve(BufferCount);
	ConstantBufferDescriptorHeaps.resize(BufferCount);
	ConstantBufferUploadHeaps.resize(BufferCount);
	ConstantBufferUploadHeapPointers.resize(BufferCount);
	for (uint32_t Index = 0; Index < BufferCount; ++Index)
	{
		CommandLists.push_back(std::make_shared<FCommandList>(Device, D3D12_COMMAND_LIST_TYPE_DIRECT, true,
		                                                      ("Command List Frame " +
			                                                      std::to_string(Index)).c_str()));

		Fences.push_back(std::make_shared<FFence>(Device));

		D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc;
		DescriptorHeapDesc.NumDescriptors = 1;
		DescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		DescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		DescriptorHeapDesc.NodeMask = 0;
		HRESULT CreateDescriptorHeapResult = Device->CreateDescriptorHeap(
			&DescriptorHeapDesc, IID_PPV_ARGS(&ConstantBufferDescriptorHeaps[Index]));
		SB_D3D_ASSERT(CreateDescriptorHeapResult, "Failed to create descriptor heap");

		CD3DX12_HEAP_PROPERTIES UploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC ConstantBufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(
			static_cast<uint64_t>(1024) * 64);
		HRESULT CreateResourceResult = Device->CreateCommittedResource(
			&UploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&ConstantBufferResourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&ConstantBufferUploadHeaps[Index]));
		SB_D3D_ASSERT(CreateResourceResult, "Failed to create constant buffer upload heap");

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = ConstantBufferUploadHeaps[Index]->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = sizeof(FConstantBuffer) + 255 & ~255;
		Device->CreateConstantBufferView(
			&cbvDesc, ConstantBufferDescriptorHeaps[Index]->GetCPUDescriptorHandleForHeapStart());

		ZeroMemory(&ConstantBuffer, sizeof(FConstantBuffer));

		CD3DX12_RANGE ReadRange(0, 0);
		HRESULT MapResult = ConstantBufferUploadHeaps[Index]->Map(0, &ReadRange,
		                                                          reinterpret_cast<void**>(&
			                                                          ConstantBufferUploadHeapPointers[
				                                                          Index]));
		SB_D3D_ASSERT(MapResult, "Failed to map constant buffer upload heap");
		memcpy(ConstantBufferUploadHeapPointers[Index], &ConstantBuffer, sizeof(FConstantBuffer));
	}

	D3D12_DESCRIPTOR_HEAP_DESC RtvDescriptorHeapDesc;
	RtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	RtvDescriptorHeapDesc.NumDescriptors = 3;
	RtvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	RtvDescriptorHeapDesc.NodeMask = 0;
	const HRESULT DescriptorHeapCreateResult = Device->CreateDescriptorHeap(
		&RtvDescriptorHeapDesc, IID_PPV_ARGS(&RtvDescriptorHeap));
	SB_D3D_ASSERT(DescriptorHeapCreateResult, "Failed to create descriptor heap");
	RtvDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	D3D12_DESCRIPTOR_HEAP_DESC SrvDescriptorHeapDesc;
	SrvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	SrvDescriptorHeapDesc.NumDescriptors = 1;
	SrvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	SrvDescriptorHeapDesc.NodeMask = 0;
	const HRESULT SrvDescriptorHeapCreateResult = Device->CreateDescriptorHeap(
		&SrvDescriptorHeapDesc, IID_PPV_ARGS(&SrvDescriptorHeap));
	SB_D3D_ASSERT(SrvDescriptorHeapCreateResult, "Failed to create descriptor heap");
	SrvDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_DESCRIPTOR_HEAP_DESC DsDescriptorHeapDesc;
	DsDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DsDescriptorHeapDesc.NumDescriptors = 1;
	DsDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DsDescriptorHeapDesc.NodeMask = 0;
	const HRESULT DsDescriptorHeapCreateResult = Device->CreateDescriptorHeap(
		&DsDescriptorHeapDesc, IID_PPV_ARGS(&DsDescriptorHeap));
	SB_D3D_ASSERT(DsDescriptorHeapCreateResult, "Failed to create descriptor heap");
	DsDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	FSwapChainDesc SwapChainDesc;
	SwapChainDesc.Width = Settings.Window->GetState().Width;
	SwapChainDesc.Height = Settings.Window->GetState().Height;
	SwapChainDesc.BufferCount = BufferCount;
	SwapChainDesc.Format = Settings.Format;
	SwapChainDesc.Window = Settings.Window;
	SwapChain = std::make_shared<FSwapChain>(this, SwapChainDesc);

	CreateDepthStencilBuffer(Settings.Window->GetState().Width, Settings.Window->GetState().Height);

	// TODO: Temporary until we have a proper asset pipeline
	{
		D3D12_DESCRIPTOR_RANGE DescriptorRanges[1];
		DescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		DescriptorRanges[0].NumDescriptors = 1;
		DescriptorRanges[0].BaseShaderRegister = 0;
		DescriptorRanges[0].RegisterSpace = 0;
		DescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		D3D12_ROOT_DESCRIPTOR_TABLE DescriptorTable;
		DescriptorTable.NumDescriptorRanges = _countof(DescriptorRanges);
		DescriptorTable.pDescriptorRanges = &DescriptorRanges[0];

		D3D12_ROOT_PARAMETER RootParameters[1];
		RootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		RootParameters[0].DescriptorTable = DescriptorTable;
		RootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		CD3DX12_ROOT_SIGNATURE_DESC RootSignatureDesc;
		RootSignatureDesc.Init(_countof(RootParameters), RootParameters, 0, nullptr,
		                       D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		                       D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		                       D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		                       D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		                       D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS);

		ComPointer<ID3DBlob> RootSignatureBlob;
		HRESULT RootSignatureSerializeResult = D3D12SerializeRootSignature(
			&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			&RootSignatureBlob, nullptr);
		SB_D3D_ASSERT(RootSignatureSerializeResult, "Failed to serialize root signature");

		HRESULT CreateRootSignatureResult = Device->CreateRootSignature(0, RootSignatureBlob->GetBufferPointer(),
		                                                                RootSignatureBlob->GetBufferSize(),
		                                                                IID_PPV_ARGS(&RootSignature));
		SB_D3D_ASSERT(CreateRootSignatureResult, "Failed to create root signature");
		RootSignatureBlob.Release();

		std::unique_ptr<FShader> VertexShader = std::make_unique<FShader>("Assets/Shaders/default.vert.hlsl",
		                                                                  EShaderType::Vertex);
		std::unique_ptr<FShader> PixelShader = std::make_unique<FShader>("Assets/Shaders/default.pixl.hlsl",
		                                                                 EShaderType::Pixel);

		D3D12_INPUT_ELEMENT_DESC InputElementDescs[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{
				"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(FVector3),
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
				0
			},
		};

		D3D12_INPUT_LAYOUT_DESC InputLayoutDesc;
		InputLayoutDesc.NumElements = _countof(InputElementDescs);
		InputLayoutDesc.pInputElementDescs = InputElementDescs;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.InputLayout = InputLayoutDesc;
		PipelineDesc.pRootSignature = RootSignature.Get();
		PipelineDesc.VS = VertexShader->GetBytecode();
		PipelineDesc.PS = PixelShader->GetBytecode();
		PipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		PipelineDesc.NumRenderTargets = 1;
		PipelineDesc.RTVFormats[0] = Settings.Format;
		PipelineDesc.SampleDesc.Count = 1;
		PipelineDesc.SampleDesc.Quality = 0;
		PipelineDesc.SampleMask = 0xffffffff;
		PipelineDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		PipelineDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		PipelineDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		PipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		PipelineDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
		PipelineDesc.NodeMask = 0;

		HRESULT PipelineCreateResult = Device->CreateGraphicsPipelineState(&PipelineDesc, IID_PPV_ARGS(&PipelineState));
		SB_D3D_ASSERT(PipelineCreateResult, "Failed to create pipeline state");

		SB_SAFE_RESET(VertexShader);
		SB_SAFE_RESET(PixelShader);

		PipelineState->SetName(L"Default pipeline state");
		RootSignature->SetName(L"Default root signature");

		std::shared_ptr<FCommandList> CommandList = CommandLists.at(0);
		CommandList->Reset();

		FVertex Vertices[] = {
			// first quad (closer to camera, blue)
			{-0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f},
			{0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f},
			{-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f},
			{0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f},

			// second quad (further from camera, green)
			{-0.75f, 0.75f, 0.7f, 0.0f, 1.0f, 0.0f, 1.0f},
			{0.0f, 0.0f, 0.7f, 0.0f, 1.0f, 0.0f, 1.0f},
			{-0.75f, 0.0f, 0.7f, 0.0f, 1.0f, 0.0f, 1.0f},
			{0.0f, 0.75f, 0.7f, 0.0f, 1.0f, 0.0f, 1.0f}
		};
		VertexBuffer = std::make_unique<FVertexBuffer>(this, Vertices, _countof(Vertices));
		VertexBuffer->Upload(CommandList->GetNativeList());

		CommandList->Reset();

		FIndex Indices[] = {
			0, 1, 2,
			0, 3, 1,
		};
		IndexBuffer = std::make_unique<FIndexBuffer>(this, Indices, _countof(Indices));
		IndexBuffer->Upload(CommandList->GetNativeList());
	}

	SetViewportAndScissor(Settings.Window->GetState().Width, Settings.Window->GetState().Height);

	// TODO: Temporary -> move to own class
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io.DisplaySize = ImVec2(static_cast<float>(Settings.Window->GetState().Width),
	                        static_cast<float>(Settings.Window->GetState().Height));

	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(Settings.Window->GetHandle());
	ImGui_ImplDX12_Init(Device, BufferCount,
	                    Settings.Format, SrvDescriptorHeap,
	                    SrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
	                    SrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
}

FGraphicsDevice::~FGraphicsDevice()
{
	Flush(BufferCount);

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	SB_SAFE_RESET(IndexBuffer);
	SB_SAFE_RESET(VertexBuffer);
	PipelineState.Release();
	RootSignature.Release();
	ConstantBufferUploadHeapPointers.clear();
	for (ComPointer<ID3D12Resource2>& ConstantBufferUploadHeap : ConstantBufferUploadHeaps)
		ConstantBufferUploadHeap.Release();
	ConstantBufferUploadHeaps.clear();
	for (ComPointer<ID3D12DescriptorHeap>& ConstantBufferDescriptorHeap : ConstantBufferDescriptorHeaps)
		ConstantBufferDescriptorHeap.Release();
	ConstantBufferDescriptorHeaps.clear();

	for (std::shared_ptr<FFence>& TestFence : Fences)
		TestFence.reset();
	Fences.clear();
	for (std::shared_ptr<FCommandList>& TestCommandList : CommandLists)
		TestCommandList.reset();
	CommandLists.clear();
	ReleaseDepthStencilBuffer();
	SB_SAFE_RESET(SwapChain);
	DepthStencilBuffer.Release();
	DsDescriptorHeap.Release();
	SrvDescriptorHeap.Release();
	RtvDescriptorHeap.Release();
	CommandQueue.Release();
	Device.Release();
	Factory.Release();
	if (Settings.bEnableDebugLayer)
	{
		DXGIDebugController->ReportLiveObjects(DXGI_DEBUG_ALL,
		                                       static_cast<DXGI_DEBUG_RLO_FLAGS>(DXGI_DEBUG_RLO_DETAIL |
			                                       DXGI_DEBUG_RLO_IGNORE_INTERNAL));
		DXGIDebugController.Release();
		DebugController.Release();
	}
}

void FGraphicsDevice::SignalAndWait() const
{
	Fences[SwapChain->GetFrameIndex()]->Signal(CommandQueue);
	Fences[SwapChain->GetFrameIndex()]->WaitOnCPU(Fences[SwapChain->GetFrameIndex()]->GetFenceValue());
}

void FGraphicsDevice::Flush(const uint32_t Count) const
{
	for (uint32_t Index = 0; Index < Count; ++Index)
		SignalAndWait();
}

void FGraphicsDevice::Resize(const uint32_t InWidth, const uint32_t InHeight)
{
	bIsResizing = true;
	Flush(BufferCount);
	SwapChain->Resize(InWidth, InHeight);
	ReleaseDepthStencilBuffer();
	CreateDepthStencilBuffer(InWidth, InHeight);
	SetViewportAndScissor(InWidth, InHeight);
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(static_cast<float>(InWidth),
	                        static_cast<float>(InHeight));
	bIsResizing = false;
}

std::shared_ptr<FVertexBuffer> FGraphicsDevice::CreateVertexBuffer(FVertex* Vertices, uint32_t Count)
{
	return std::make_shared<FVertexBuffer>(this, Vertices, Count);
}

std::shared_ptr<FIndexBuffer> FGraphicsDevice::CreateIndexBuffer(FIndex* Indices, uint32_t Count)
{
	return std::make_shared<FIndexBuffer>(this, Indices, Count);
}

void FGraphicsDevice::BeginFrame(const FClearColor& ClearColor)
{
	if (bIsResizing)
		return;
	const uint32_t FrameIndex = SwapChain->GetFrameIndex();

	const std::shared_ptr<FCommandList> CommandListWrapper = CommandLists[FrameIndex];
	ComPointer<ID3D12GraphicsCommandList7> CommandList = CommandListWrapper->GetNativeList();
	CommandListWrapper->Reset();

	D3D12_RESOURCE_BARRIER Barrier;
	Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	Barrier.Transition.pResource = SwapChain->GetBackBuffer().Get();
	Barrier.Transition.Subresource = 0;
	Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	CommandList->ResourceBarrier(1, &Barrier);


	const D3D12_CPU_DESCRIPTOR_HANDLE BackBufferDescriptor = SwapChain->GetBackBufferDescriptor();
	const CD3DX12_CPU_DESCRIPTOR_HANDLE DsvHandle(DsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	CommandList->OMSetRenderTargets(1, &BackBufferDescriptor, FALSE, &DsvHandle);

	const float ClearColorArray[] = {ClearColor.Red, ClearColor.Green, ClearColor.Blue, ClearColor.Alpha};
	CommandList->ClearRenderTargetView(BackBufferDescriptor, ClearColorArray, 0, nullptr);
	CommandList->ClearDepthStencilView(DsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH,
	                                   1.0f, 0, 0, nullptr);

	memcpy(ConstantBufferUploadHeapPointers[FrameIndex], &ConstantBuffer, sizeof(FConstantBuffer));

	ID3D12DescriptorHeap* DescriptorHeaps[] = {
		ConstantBufferDescriptorHeaps[FrameIndex].Get(),
	};
	CommandList->SetDescriptorHeaps(_countof(DescriptorHeaps), DescriptorHeaps);

	CommandList->SetGraphicsRootSignature(RootSignature.Get());
	CommandList->SetPipelineState(PipelineState.Get());
	CommandList->SetGraphicsRootDescriptorTable(
		0, ConstantBufferDescriptorHeaps[FrameIndex]->GetGPUDescriptorHandleForHeapStart());

	CommandList->RSSetViewports(1, &Viewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);
	CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	FDrawCall DrawCall;
	DrawCall.VertexBuffer = VertexBuffer;
	DrawCall.IndexBuffer = IndexBuffer;
	Draw(DrawCall);
	CommandList->DrawIndexedInstanced(6, 1, 0, 4, 0);

	ID3D12DescriptorHeap* SrvDescriptorHeaps[] = {
		SrvDescriptorHeap.Get(),
	};
	CommandList->SetDescriptorHeaps(_countof(SrvDescriptorHeaps), SrvDescriptorHeaps);

	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Debug");
	ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);
	static float FPS[1000];
	static int FPSIndex = 0;
	FPS[FPSIndex] = ImGui::GetIO().Framerate;
	FPSIndex = (FPSIndex + 1) % 1000;
	ImGui::PlotLines("FPS", FPS, 1000, FPSIndex, nullptr, 0.0f, 1300.0f, ImVec2(0, 80));
	ImGui::SliderFloat("Offset X", &ConstantBuffer.OffsetX, -1.0f, 1.0f);
	ImGui::SliderFloat("Offset Y", &ConstantBuffer.OffsetY, -1.0f, 1.0f);
	ImGui::SliderFloat("Offset Z", &ConstantBuffer.OffsetZ, -1.0f, 1.0f);
	ImGui::End();
}

void FGraphicsDevice::EndFrame() const
{
	if (bIsResizing)
		return;
	const uint32_t FrameIndex = SwapChain->GetFrameIndex();

	const std::shared_ptr<FCommandList> CommandListWrapper = CommandLists[FrameIndex];
	ComPointer<ID3D12GraphicsCommandList7> CommandList = CommandListWrapper->GetNativeList();

	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), CommandList);

	D3D12_RESOURCE_BARRIER Barrier;
	Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	Barrier.Transition.pResource = SwapChain->GetBackBuffer().Get();
	Barrier.Transition.Subresource = 0;
	Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	CommandList->ResourceBarrier(1, &Barrier);

	CommandListWrapper->Execute(CommandQueue, Fences[FrameIndex]);
	SwapChain->Present(true);
}

void FGraphicsDevice::Draw(const FDrawCall& DrawCall) const
{
	ComPointer<ID3D12GraphicsCommandList7> CommandList = CommandLists[SwapChain->GetFrameIndex()]->GetNativeList();
	const D3D12_VERTEX_BUFFER_VIEW VertexBufferView = DrawCall.VertexBuffer->GetView();
	CommandList->IASetVertexBuffers(0, 1, &VertexBufferView);
	const D3D12_INDEX_BUFFER_VIEW IndexBufferView = DrawCall.IndexBuffer->GetView();
	CommandList->IASetIndexBuffer(&IndexBufferView);
	CommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void FGraphicsDevice::ExecuteCommandList(ComPointer<ID3D12GraphicsCommandList7> CommandList)
{
	ID3D12CommandList* ExecuteCommandLists[] = {CommandList.Get()};
	CommandQueue->ExecuteCommandLists(_countof(ExecuteCommandLists), ExecuteCommandLists);
	SignalAndWait();
}

D3D12_SHADER_BYTECODE FGraphicsDevice::CompileShader(const std::wstring& FileName, const std::string& EntryPoint,
                                                     const std::string& Target, ComPointer<ID3DBlob>& ShaderBlob) const
{
	ComPointer<ID3DBlob> ErrorBlob;
	const HRESULT CompileResult = D3DCompileFromFile(FileName.c_str(), nullptr, nullptr, EntryPoint.c_str(),
	                                                 Target.c_str(), 0, 0, &ShaderBlob, &ErrorBlob);
	if (FAILED(CompileResult))
	{
		if (ErrorBlob)
		{
			std::string ErrorString(static_cast<char*>(ErrorBlob->GetBufferPointer()));
			SB_LOG_ERROR("Failed to compile shader '{}': {}", std::filesystem::path(FileName).filename().string(),
			             ErrorString);
		}
		else
		{
			SB_LOG_ERROR("Failed to compile shader '{}'", std::filesystem::path(FileName).filename().string());
		}
	}
	SB_D3D_ASSERT(CompileResult, "Failed to compile shader");
	D3D12_SHADER_BYTECODE ShaderBytecode;
	ShaderBytecode.BytecodeLength = ShaderBlob->GetBufferSize();
	ShaderBytecode.pShaderBytecode = ShaderBlob->GetBufferPointer();
	return ShaderBytecode;
}

void FGraphicsDevice::SetViewportAndScissor(const uint32_t Width, const uint32_t Height)
{
	Viewport.TopLeftX = 0;
	Viewport.TopLeftY = 0;
	Viewport.Width = static_cast<float>(Width);
	Viewport.Height = static_cast<float>(Height);
	Viewport.MinDepth = 0.0f;
	Viewport.MaxDepth = 1.0f;
	ScissorRect.left = 0;
	ScissorRect.top = 0;
	ScissorRect.right = Width;
	ScissorRect.bottom = Height;
}

void FGraphicsDevice::CreateDepthStencilBuffer(const uint32_t Width, const uint32_t Height)
{
	D3D12_CLEAR_VALUE DepthStencilClearValue;
	DepthStencilClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	DepthStencilClearValue.DepthStencil.Depth = 1.0f;
	DepthStencilClearValue.DepthStencil.Stencil = 0;

	const CD3DX12_HEAP_PROPERTIES DepthStencilHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
	const CD3DX12_RESOURCE_DESC DepthStencilResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT,
		Width,
		Height,
		1, 1, 1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
	const HRESULT CreateDepthStencilResult = Device->CreateCommittedResource(&DepthStencilHeapProperties,
	                                                                         D3D12_HEAP_FLAG_NONE,
	                                                                         &DepthStencilResourceDesc,
	                                                                         D3D12_RESOURCE_STATE_DEPTH_WRITE,
	                                                                         &DepthStencilClearValue,
	                                                                         IID_PPV_ARGS(&DepthStencilBuffer));
	SB_D3D_ASSERT(CreateDepthStencilResult, "Failed to create depth stencil buffer");

	D3D12_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc;
	DepthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	DepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	DepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;
	DepthStencilViewDesc.Texture2D.MipSlice = 0;
	Device->CreateDepthStencilView(DepthStencilBuffer.Get(), &DepthStencilViewDesc,
	                               DsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

void FGraphicsDevice::ReleaseDepthStencilBuffer()
{
	DepthStencilBuffer.Release();
}
