#include "pch.h"

#include <Engine/Graphics/GraphicsDevice.h>

#include <filesystem>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx12.h>

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

	CommandAllocators.resize(BufferCount);
	CommandLists.resize(BufferCount);
	Fences.resize(BufferCount);
	FenceValues.resize(BufferCount);
	for (uint32_t Index = 0; Index < BufferCount; ++Index)
	{
		const HRESULT CommandAllocatorCreateResult = Device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocators[Index]));
		SB_D3D_ASSERT(CommandAllocatorCreateResult, "Failed to create command allocator");
		CommandAllocators[Index]->SetName((L"Command Allocator Frame " + std::to_wstring(Index)).c_str());

		const HRESULT CommandListCreateResult = Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		                                                                  CommandAllocators[Index].Get(), nullptr,
		                                                                  IID_PPV_ARGS(&CommandLists[Index]));
		SB_D3D_ASSERT(CommandListCreateResult, "Failed to create command list");
		CommandLists[Index]->Close();
		CommandLists[Index]->SetName((L"Command List Frame " + std::to_wstring(Index)).c_str());

		const HRESULT FenceCreateResult = Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fences[Index]));
		SB_D3D_ASSERT(FenceCreateResult, "Failed to create fence");
		Fences[Index]->SetName((L"Fence Frame " + std::to_wstring(Index)).c_str());
		FenceValues[Index] = 0;
	}
	FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	SB_ASSERT_CRITICAL(FenceEvent != nullptr, E_FAIL, "Failed to create fence event");

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

	FSwapChainDesc SwapChainDesc;
	SwapChainDesc.Width = Settings.Window->GetState().Width;
	SwapChainDesc.Height = Settings.Window->GetState().Height;
	SwapChainDesc.BufferCount = BufferCount;
	SwapChainDesc.Format = Settings.Format;
	SwapChainDesc.Window = Settings.Window;
	SwapChain = std::make_shared<FSwapChain>(this, SwapChainDesc);

	CD3DX12_ROOT_SIGNATURE_DESC RootSignatureDesc;
	RootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPointer<ID3DBlob> RootSignatureBlob;
	HRESULT RootSignatureSerializeResult = D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
	                                                                   &RootSignatureBlob, nullptr);
	SB_D3D_ASSERT(RootSignatureSerializeResult, "Failed to serialize root signature");

	HRESULT CreateRootSignatureResult = Device->CreateRootSignature(0, RootSignatureBlob->GetBufferPointer(),
	                                                                RootSignatureBlob->GetBufferSize(),
	                                                                IID_PPV_ARGS(&RootSignature));
	SB_D3D_ASSERT(CreateRootSignatureResult, "Failed to create root signature");
	RootSignatureBlob.Release();

	ComPointer<ID3DBlob> VertexShaderBlob;
	ComPointer<ID3DBlob> PixelShaderBlob;
	D3D12_SHADER_BYTECODE VertexShaderByteCode = CompileShader(L"Assets/Shaders/default.vert.hlsl", "main", "vs_5_0",
	                                                           VertexShaderBlob);
	D3D12_SHADER_BYTECODE PixelShaderByteCode = CompileShader(L"Assets/Shaders/default.pixl.hlsl", "main", "ps_5_0",
	                                                          PixelShaderBlob);

	D3D12_INPUT_ELEMENT_DESC InputElementDescs[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{
			"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(FVector3), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
	};

	D3D12_INPUT_LAYOUT_DESC InputLayoutDesc;
	InputLayoutDesc.NumElements = _countof(InputElementDescs);
	InputLayoutDesc.pInputElementDescs = InputElementDescs;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
	PipelineDesc.InputLayout = InputLayoutDesc;
	PipelineDesc.pRootSignature = RootSignature.Get();
	PipelineDesc.VS = VertexShaderByteCode;
	PipelineDesc.PS = PixelShaderByteCode;
	PipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	PipelineDesc.NumRenderTargets = 1;
	PipelineDesc.RTVFormats[0] = Settings.Format;
	PipelineDesc.SampleDesc.Count = 1;
	PipelineDesc.SampleDesc.Quality = 0;
	PipelineDesc.SampleMask = 0xffffffff;
	PipelineDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	PipelineDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	// PipelineDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	PipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	PipelineDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	PipelineDesc.NodeMask = 0;

	HRESULT PipelineCreateResult = Device->CreateGraphicsPipelineState(&PipelineDesc, IID_PPV_ARGS(&PipelineState));
	SB_D3D_ASSERT(PipelineCreateResult, "Failed to create pipeline state");
	VertexShaderBlob.Release();
	PixelShaderBlob.Release();
	PipelineState->SetName(L"Default pipeline state");
	RootSignature->SetName(L"Default root signature");

	ComPointer<ID3D12GraphicsCommandList7> CommandList = CommandLists.at(0);
	CommandList->Reset(CommandAllocators.at(0).Get(), nullptr);

	FVertex Vertices[] = {
		{{-0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}}, // top left
		{{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}}, // bottom right
		{{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}, // bottom left
		{{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}}, // top right
	};
	VertexBuffer = std::make_unique<FVertexBuffer>(this, Vertices, _countof(Vertices));
	VertexBuffer->Upload(CommandList);

	CommandList->Reset(CommandAllocators.at(0).Get(), nullptr);

	FIndex Indices[] = {
		0, 1, 2,
		0, 3, 1,
	};
	IndexBuffer = std::make_unique<FIndexBuffer>(this, Indices, _countof(Indices));
	IndexBuffer->Upload(CommandList);

	SetViewportAndScissor(Settings.Window->GetState().Width, Settings.Window->GetState().Height);

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
	if (FenceEvent)
		CloseHandle(FenceEvent);
	FenceValues.clear();
	for (ComPointer<ID3D12Fence1>& Fence : Fences)
		Fence.Release();
	Fences.clear();
	for (ComPointer<ID3D12GraphicsCommandList7>& CommandList : CommandLists)
		CommandList.Release();
	CommandLists.clear();
	for (ComPointer<ID3D12CommandAllocator>& CommandAllocator : CommandAllocators)
		CommandAllocator.Release();
	CommandAllocators.clear();
	SB_SAFE_RESET(SwapChain);
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

void FGraphicsDevice::SignalAndWait()
{
	ComPointer<ID3D12Fence1> Fence = Fences[SwapChain->GetFrameIndex()];
	FenceValues[SwapChain->GetFrameIndex()]++;
	CommandQueue->Signal(Fence, FenceValues[SwapChain->GetFrameIndex()]);
	const HRESULT WaitResult = Fence->SetEventOnCompletion(FenceValues[SwapChain->GetFrameIndex()], FenceEvent);
	const DWORD WaitForSingleObjectResult = WaitForSingleObject(FenceEvent, 20000);
	SB_ASSERT_CRITICAL(WaitForSingleObjectResult == WAIT_OBJECT_0, E_FAIL, "Failed to wait for fence");
	SB_D3D_ASSERT(WaitResult, "Failed to set event on completion");
}

void FGraphicsDevice::Flush(const uint32_t Count)
{
	for (uint32_t Index = 0; Index < Count; ++Index)
		SignalAndWait();
}

void FGraphicsDevice::Resize(const uint32_t InWidth, const uint32_t InHeight)
{
	Flush(BufferCount);
	SwapChain->Resize(InWidth, InHeight);
	SetViewportAndScissor(InWidth, InHeight);
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(static_cast<float>(Settings.Window->GetState().Width),
	                        static_cast<float>(Settings.Window->GetState().Height));
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
	ComPointer<ID3D12CommandAllocator> CommandAllocator = CommandAllocators[SwapChain->GetFrameIndex()];
	const HRESULT ResetCommandAllocatorResult = CommandAllocator->Reset();
	SB_D3D_ASSERT(ResetCommandAllocatorResult, "Failed to reset command allocator");

	ComPointer<ID3D12GraphicsCommandList7> CommandList = CommandLists[SwapChain->GetFrameIndex()];
	const HRESULT ResetCommandListResult = CommandList->Reset(CommandAllocator.Get(), nullptr);
	SB_D3D_ASSERT(ResetCommandListResult, "Failed to reset command list");

	D3D12_RESOURCE_BARRIER Barrier;
	Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	Barrier.Transition.pResource = SwapChain->GetBackBuffer().Get();
	Barrier.Transition.Subresource = 0;
	Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	CommandList->ResourceBarrier(1, &Barrier);

	const float ClearColorArray[] = {ClearColor.Red, ClearColor.Green, ClearColor.Blue, ClearColor.Alpha};
	const D3D12_CPU_DESCRIPTOR_HANDLE BackBufferDescriptor = SwapChain->GetBackBufferDescriptor();
	CommandList->ClearRenderTargetView(BackBufferDescriptor, ClearColorArray, 0, nullptr);
	CommandList->OMSetRenderTargets(1, &BackBufferDescriptor, FALSE, nullptr);

	CommandList->SetDescriptorHeaps(1, &SrvDescriptorHeap);

	CommandList->RSSetViewports(1, &Viewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);
	CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Snowbite");
	{
		ImGui::Text(std::format("FPS: {}", ImGui::GetIO().Framerate).c_str());
		ImGui::Text(std::format("Frame time: {} ms", 1000.0f / ImGui::GetIO().Framerate).c_str());
		ImGui::Text(std::format("Buffering mode: {}", GetBufferingModeName(Settings.BufferingMode)).c_str());
		ImGui::Text(std::format("Window size: {}x{}", Settings.Window->GetState().Width,
		                        Settings.Window->GetState().Height).c_str());
	}
	ImGui::End();

	FDrawCall DrawCall;
	DrawCall.RootSignature = RootSignature;
	DrawCall.PipelineState = PipelineState;
	DrawCall.VertexBuffer = VertexBuffer;
	DrawCall.IndexBuffer = IndexBuffer;
	Draw(DrawCall);
}

void FGraphicsDevice::EndFrame()
{
	ComPointer<ID3D12GraphicsCommandList7> CommandList = CommandLists[SwapChain->GetFrameIndex()];

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

	const HRESULT CloseCommandListResult = CommandList->Close();
	SB_D3D_ASSERT(CloseCommandListResult, "Failed to close command list");

	ExecuteCommandList(CommandList);
	SwapChain->Present(true);
}

void FGraphicsDevice::Draw(FDrawCall& DrawCall) const
{
	ComPointer<ID3D12GraphicsCommandList7> CommandList = CommandLists[SwapChain->GetFrameIndex()];
	CommandList->SetGraphicsRootSignature(DrawCall.RootSignature.Get());
	CommandList->SetPipelineState(DrawCall.PipelineState.Get());
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
