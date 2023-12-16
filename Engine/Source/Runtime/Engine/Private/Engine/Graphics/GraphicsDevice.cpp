#include "pch.h"

#include <codecvt>
#include <filesystem>
#include <Engine/Graphics/GraphicsDevice.h>

// ReSharper disable CppNonInlineVariableDefinitionInHeaderFile
extern "C" {
// http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;

//  https://gpuopen.com/amdpowerxpressrequesthighperformance/
_declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001;
}

// ReSharper enable CppNonInlineVariableDefinitionInHeaderFile

FGraphicsDevice::FGraphicsDevice(const FGraphicsDeviceSettings& InSettings)
	: Settings(InSettings)
{
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

	uint32_t BufferCount = static_cast<uint32_t>(Settings.BufferingMode);
	CommandAllocators.resize(BufferCount);
	CommandLists.resize(BufferCount);
	Fences.resize(BufferCount);
	FenceValues.resize(BufferCount);
	for (uint32_t Index = 0; Index < BufferCount; ++Index)
	{
		const HRESULT CommandAllocatorCreateResult = Device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocators[Index]));
		SB_D3D_ASSERT(CommandAllocatorCreateResult, "Failed to create command allocator");

		const HRESULT CommandListCreateResult = Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		                                                                  CommandAllocators[Index].Get(), nullptr,
		                                                                  IID_PPV_ARGS(&CommandLists[Index]));
		SB_D3D_ASSERT(CommandListCreateResult, "Failed to create command list");
		CommandLists[Index]->Close();

		const HRESULT FenceCreateResult = Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fences[Index]));
		SB_D3D_ASSERT(FenceCreateResult, "Failed to create fence");
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

	FSwapChainDesc SwapChainDesc;
	SwapChainDesc.Width = 1280;
	SwapChainDesc.Height = 720;
	SwapChainDesc.BufferCount = static_cast<uint32_t>(Settings.BufferingMode);
	SwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
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

	D3D12_SHADER_BYTECODE VertexShaderByteCode = CompileShader(L"Assets/Shaders/default.vert.hlsl", "main", "vs_5_0",
	                                                           VertexShaderBlob);
	D3D12_SHADER_BYTECODE PixelShaderByteCode = CompileShader(L"Assets/Shaders/default.pixl.hlsl", "main", "ps_5_0",
	                                                          PixelShaderBlob);

	D3D12_INPUT_ELEMENT_DESC InputElementDescs[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
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
	PipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
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

	struct Vector3
	{
		float X = 0.0f;
		float Y = 0.0f;
		float Z = 0.0f;
	};

	struct Vertex
	{
		Vector3 Position;
		Vector3 Color;
	};

	Vertex Vertices[] = {
		{{-0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}}, // top left
		{{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}}, // bottom right
		{{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}, // bottom left
		{{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}, // top right
	};
	size_t VerticesSize = sizeof(Vertices);
	CD3DX12_HEAP_PROPERTIES HeapProperties(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(VerticesSize);
	HRESULT VertexBufferCreateResult = Device->CreateCommittedResource(
		&HeapProperties, D3D12_HEAP_FLAG_NONE,
		&ResourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr,
		IID_PPV_ARGS(&VertexBuffer));
	SB_D3D_ASSERT(VertexBufferCreateResult, "Failed to create vertex buffer");
	VertexBuffer->SetName(L"Vertex Buffer Resource Heap");

	ComPointer<ID3D12Resource> VertexBufferUploadHeap;
	CD3DX12_HEAP_PROPERTIES UploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC ResourceDesc2 = CD3DX12_RESOURCE_DESC::Buffer(VerticesSize);
	HRESULT VertexBufferUploadHeapCreateResult = Device->CreateCommittedResource(
		&UploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&ResourceDesc2, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&VertexBufferUploadHeap));
	SB_D3D_ASSERT(VertexBufferUploadHeapCreateResult, "Failed to create vertex buffer upload heap");
	VertexBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

	D3D12_SUBRESOURCE_DATA VertexData = {};
	VertexData.pData = reinterpret_cast<BYTE*>(Vertices);
	VertexData.RowPitch = VerticesSize;
	VertexData.SlicePitch = VerticesSize;

	ComPointer<ID3D12GraphicsCommandList7> CommandList = CommandLists.at(0);
	CommandList->Reset(CommandAllocators.at(0).Get(), nullptr);

	UpdateSubresources(CommandList.Get(), VertexBuffer.Get(), VertexBufferUploadHeap.Get(), 0, 0, 1, &VertexData);
	CD3DX12_RESOURCE_BARRIER Barrier = CD3DX12_RESOURCE_BARRIER::Transition(VertexBuffer,
	                                                                        D3D12_RESOURCE_STATE_COPY_DEST,
	                                                                        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	CommandList->ResourceBarrier(1, &Barrier);
	CommandList->Close();
	ExecuteCommandList(CommandList);
	VertexBufferView.BufferLocation = VertexBuffer->GetGPUVirtualAddress();
	VertexBufferView.StrideInBytes = sizeof(Vertex);
	VertexBufferView.SizeInBytes = VerticesSize;

	DWORD Indices[] = {
		0, 1, 2, // first triangle
		0, 3, 1 // second triangle
	};
	size_t IndicesSize = sizeof(Indices);
	CD3DX12_RESOURCE_DESC IndexBufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(IndicesSize);
	CD3DX12_HEAP_PROPERTIES IndexBufferHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	HRESULT IndexBufferCreateResult = Device->CreateCommittedResource(
		&IndexBufferHeapProperties, D3D12_HEAP_FLAG_NONE,
		&IndexBufferResourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr,
		IID_PPV_ARGS(&IndexBuffer));
	SB_D3D_ASSERT(IndexBufferCreateResult, "Failed to create index buffer");
	IndexBuffer->SetName(L"Index Buffer Resource Heap");

	ComPointer<ID3D12Resource> IndexBufferUploadHeap;
	CD3DX12_HEAP_PROPERTIES IndexBufferUploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC IndexBufferUploadResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(IndicesSize);
	HRESULT IndexBufferUploadHeapCreateResult = Device->CreateCommittedResource(
		&IndexBufferUploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&IndexBufferUploadResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&IndexBufferUploadHeap));
	SB_D3D_ASSERT(IndexBufferUploadHeapCreateResult, "Failed to create index buffer upload heap");
	IndexBufferUploadHeap->SetName(L"Index Buffer Upload Resource Heap");

	D3D12_SUBRESOURCE_DATA IndexData = {};
	IndexData.pData = reinterpret_cast<BYTE*>(Indices);
	IndexData.RowPitch = IndicesSize;
	IndexData.SlicePitch = IndicesSize;

	CommandList->Reset(CommandAllocators.at(0).Get(), nullptr);

	UpdateSubresources(CommandList.Get(), IndexBuffer.Get(), IndexBufferUploadHeap.Get(), 0, 0, 1, &IndexData);
	CD3DX12_RESOURCE_BARRIER IndexBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(IndexBuffer,
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	CommandList->ResourceBarrier(1, &IndexBufferBarrier);
	CommandList->Close();
	ExecuteCommandList(CommandList);
	IndexBufferView.BufferLocation = IndexBuffer->GetGPUVirtualAddress();
	IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	IndexBufferView.SizeInBytes = IndicesSize;

	Viewport.TopLeftX = 0;
	Viewport.TopLeftY = 0;
	Viewport.Width = Settings.Window->GetState().Width;
	Viewport.Height = Settings.Window->GetState().Height;
	Viewport.MinDepth = 0.0f;
	Viewport.MaxDepth = 1.0f;

	ScissorRect.left = 0;
	ScissorRect.top = 0;
	ScissorRect.right = Settings.Window->GetState().Width;
	ScissorRect.bottom = Settings.Window->GetState().Height;
}

FGraphicsDevice::~FGraphicsDevice()
{
	Flush(static_cast<uint32_t>(Settings.BufferingMode));
	VertexBuffer.Release();
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
	Flush(static_cast<uint32_t>(Settings.BufferingMode));
	SwapChain->Resize(InWidth, InHeight);
	Viewport.TopLeftX = 0;
	Viewport.TopLeftY = 0;
	Viewport.Width = Settings.Window->GetState().Width;
	Viewport.Height = Settings.Window->GetState().Height;
	Viewport.MinDepth = 0.0f;
	Viewport.MaxDepth = 1.0f;

	ScissorRect.left = 0;
	ScissorRect.top = 0;
	ScissorRect.right = Settings.Window->GetState().Width;
	ScissorRect.bottom = Settings.Window->GetState().Height;
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

	CommandList->SetGraphicsRootSignature(RootSignature.Get());
	CommandList->SetPipelineState(PipelineState.Get());
	CommandList->RSSetViewports(1, &Viewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);
	CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	CommandList->IASetVertexBuffers(0, 1, &VertexBufferView);
	CommandList->IASetIndexBuffer(&IndexBufferView);
	CommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void FGraphicsDevice::EndFrame()
{
	ComPointer<ID3D12GraphicsCommandList7> CommandList = CommandLists[SwapChain->GetFrameIndex()];

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
