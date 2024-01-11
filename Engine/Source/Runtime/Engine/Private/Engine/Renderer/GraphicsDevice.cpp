#include "pch.h"

#include <Engine/Renderer/GraphicsDevice.h>

FGraphicsDevice::FGraphicsDevice()
{
}

FGraphicsDevice::~FGraphicsDevice()
{
}

HRESULT FGraphicsDevice::Initialize(const std::shared_ptr<FWindow>& InWindow, const FGraphicsDeviceDesc& InDesc)
{
	Window = InWindow;
	Desc = InDesc;
#ifdef SB_DEBUG
	EnableDebugLayer();
	uint32_t FactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#else
	uint32_t FactoryFlags = 0;
#endif
	SB_CHECK(CreateDXGIFactory2(FactoryFlags, IID_PPV_ARGS(&Factory)));
	ComPtr<ID3D12DeviceFactory> DeviceFactory;
	SB_CHECK(InitializeSDK(DeviceFactory));
	constexpr D3D_FEATURE_LEVEL RequiredFeatureLevel = D3D_FEATURE_LEVEL_12_2;
	SB_CHECK(ChooseAdapter(DeviceFactory, RequiredFeatureLevel, Adapter));
	SB_CHECK(DeviceFactory->CreateDevice(Adapter.Get(), RequiredFeatureLevel, IID_PPV_ARGS(&Device)));
	SB_SAFE_COM_RESET(DeviceFactory);

	D3D12_COMMAND_QUEUE_DESC GraphicsCommandQueueDesc;
	GraphicsCommandQueueDesc.NodeMask = 0;
	GraphicsCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	GraphicsCommandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
	GraphicsCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	SB_CHECK(Device->CreateCommandQueue(&GraphicsCommandQueueDesc, IID_PPV_ARGS(&GraphicsCommandQueue)));
	SB_CHECK(GraphicsCommandQueue->SetName(L"GraphicsCommandQueue"));

	if (Desc.MaxRenderTargets < BufferCount)
	{
		SB_LOG_ERROR("MaxRenderTargets is lower than the buffer count!");
		return E_FAIL;
	}

	D3D12_DESCRIPTOR_HEAP_DESC RTVHeapDesc;
	RTVHeapDesc.NodeMask = 0;
	RTVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	RTVHeapDesc.NumDescriptors = Desc.MaxRenderTargets;
	RTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	SB_CHECK(Device->CreateDescriptorHeap(&RTVHeapDesc, IID_PPV_ARGS(&RTVHeap)));
	SB_LOG_INFO("Max Render Targets: {}", Desc.MaxRenderTargets);

	FSwapChainDesc SwapChainDesc;
	SwapChainDesc.Window = Window;
	SwapChainDesc.Factory = Factory;
	SwapChainDesc.Device = Device;
	SwapChainDesc.CommandQueue = GraphicsCommandQueue;
	SwapChainDesc.RTVHeap = RTVHeap;
	SwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChainDesc.BufferCount = 3;
	SwapChain = std::make_shared<FSwapChain>();
	SB_CHECK(SwapChain->Initialize(SwapChainDesc));

	for (int Index = 0; Index < BufferCount; ++Index)
	{
		FFrameResource& FrameResource = FrameResources[Index];
		SB_CHECK(
			Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&FrameResource.CommandAllocator)
			));
		SB_CHECK(FrameResource.CommandAllocator->SetName(
			StringToWideString("CommandAllocatorFrame" + std::to_string(Index)).c_str()));
		SB_CHECK(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, FrameResource.CommandAllocator.Get(),
			nullptr, IID_PPV_ARGS(&FrameResource.CommandList)));
		SB_CHECK(
			FrameResource.CommandList->SetName(StringToWideString("CommandListFrame" + std::to_string(Index)).c_str()));
		SB_CHECK(FrameResource.CommandList->Close());
		SB_CHECK(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&FrameResource.Fence)));
		SB_CHECK(FrameResource.Fence->SetName(StringToWideString("CommandFenceFrame" + std::to_string(Index)).c_str()));
		FrameResource.FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	}
	return S_OK;
}

void FGraphicsDevice::Destroy()
{
	FlushFrames();
	for (int Index = 0; Index < BufferCount; ++Index)
	{
		FFrameResource& FrameResource = FrameResources[Index];
		FrameResource.CommandAllocator.Reset();
		FrameResource.CommandList.Reset();
		FrameResource.Fence.Reset();
		CloseHandle(FrameResource.FenceEvent);
	}
	FrameResources.fill({});
	SB_SAFE_DESTROY(SwapChain);
	SB_SAFE_COM_RESET(RTVHeap);
	SB_SAFE_COM_RESET(GraphicsCommandQueue);
	SB_SAFE_COM_RESET(Device);
	SB_SAFE_COM_RESET(Adapter);
	SB_SAFE_COM_RESET(Factory);
#ifdef SB_DEBUG
	if (DebugController)
	{
		SB_LOG_WARN("Reporting live objects. Take a look at the debug output to see if there are any leaks.");
		OutputDebugString(TEXT("Reporting live objects:\n"));
		DebugController->ReportLiveObjects(DXGI_DEBUG_ALL,
		                                   static_cast<DXGI_DEBUG_RLO_FLAGS>(DXGI_DEBUG_RLO_DETAIL |
			                                   DXGI_DEBUG_RLO_IGNORE_INTERNAL));
		SB_SAFE_COM_RESET(DebugController);
	}
#endif
	Window.reset();
}

HRESULT FGraphicsDevice::BeginFrame()
{
	CurrentFrameIndex = SwapChain->GetBackBufferIndex();
	FFrameResource& FrameResource = FrameResources.at(CurrentFrameIndex);
	const ComPtr<ID3D12GraphicsCommandList9> CommandList = FrameResource.CommandList;

	if (FrameResource.Fence->GetCompletedValue() < FrameResource.FenceValue)
	{
		SB_CHECK(FrameResource.Fence->SetEventOnCompletion(FrameResource.FenceValue, FrameResource.FenceEvent));
		const DWORD WaitResult = WaitForSingleObject(FrameResource.FenceEvent, 20000);
		if (WaitResult != WAIT_OBJECT_0)
			return DXGI_ERROR_WAIT_TIMEOUT;
	}
	FrameResource.FenceValue++;

	SB_CHECK(FrameResource.CommandAllocator->Reset());
	SB_CHECK(CommandList->Reset(FrameResource.CommandAllocator.Get(), nullptr));

	const CD3DX12_RESOURCE_BARRIER RenderTargetBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		SwapChain->GetBackBuffer(CurrentFrameIndex).Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	CommandList->ResourceBarrier(1, &RenderTargetBarrier);

	const D3D12_CPU_DESCRIPTOR_HANDLE RTVHandle = SwapChain->GetBackBufferHandle(CurrentFrameIndex);
	CommandList->OMSetRenderTargets(1, &RTVHandle, false, nullptr);
	constexpr float ClearColor[] = {0.08f, 0.08f, 0.08f, 1.0f};
	CommandList->ClearRenderTargetView(RTVHandle, ClearColor, 0, nullptr);
	return S_OK;
}

HRESULT FGraphicsDevice::EndFrame() const
{
	const FFrameResource& FrameResource = FrameResources.at(CurrentFrameIndex);
	const ComPtr<ID3D12GraphicsCommandList9> CommandList = FrameResource.CommandList;

	const CD3DX12_RESOURCE_BARRIER RenderTargetBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		SwapChain->GetBackBuffer(CurrentFrameIndex).Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	CommandList->ResourceBarrier(1, &RenderTargetBarrier);

	SB_CHECK(CommandList->Close());

	ID3D12CommandList* CommandLists[] = {CommandList.Get()};
	GraphicsCommandQueue->ExecuteCommandLists(_countof(CommandLists), CommandLists);

	SB_CHECK(SwapChain->Present(true));

	SB_CHECK(GraphicsCommandQueue->Signal(FrameResource.Fence.Get(), FrameResource.FenceValue));
	return S_OK;
}

HRESULT FGraphicsDevice::Resize(const uint32_t NewWidth, const uint32_t NewHeight)
{
	FlushFrames();
	SB_CHECK(SwapChain->Resize(NewWidth, NewHeight));
	return S_OK;
}

HRESULT FGraphicsDevice::FlushFrames()
{
	for (int Index = 0; Index < BufferCount; ++Index)
	{
		FFrameResource& FrameResource = FrameResources.at(Index);
		FrameResource.FenceValue++;
		SB_CHECK(FrameResource.Fence->SetEventOnCompletion(FrameResource.FenceValue, FrameResource.FenceEvent));
		SB_CHECK(GraphicsCommandQueue->Signal(FrameResource.Fence.Get(), FrameResource.FenceValue));
		const DWORD WaitResult = WaitForSingleObject(FrameResource.FenceEvent, 20000);
		if (WaitResult != WAIT_OBJECT_0)
			return DXGI_ERROR_WAIT_TIMEOUT;
	}
	return S_OK;
}

#ifdef SB_DEBUG
void FGraphicsDevice::EnableDebugLayer()
{
	const HRESULT DebugResult = DXGIGetDebugInterface1(0, IID_PPV_ARGS(&DebugController));
	if (SUCCEEDED(DebugResult))
		DebugController->EnableLeakTrackingForThread();
	else
		SB_LOG_WARN("Failed to enbable dxgi debug layer");
	ComPtr<ID3D12Debug6> DebugInterface;
	const HRESULT D3D12DebugResult = D3D12GetDebugInterface(IID_PPV_ARGS(&DebugInterface));
	if (SUCCEEDED(D3D12DebugResult))
	{
		DebugInterface->EnableDebugLayer();
		DebugInterface->SetEnableGPUBasedValidation(true);
		DebugInterface->SetEnableAutoName(true);
	}
	SB_SAFE_COM_RESET(DebugInterface);
}
#endif

HRESULT FGraphicsDevice::InitializeSDK(ComPtr<ID3D12DeviceFactory>& DeviceFactory) const
{
	ComPtr<ID3D12SDKConfiguration1> SDKConfig;
	constexpr uint32_t SDKVersion = 611;
	const char* SDKPath = ".\\D3D12\\";
	SB_CHECK(D3D12GetInterface(CLSID_D3D12SDKConfiguration, IID_PPV_ARGS(&SDKConfig)));
	SB_CHECK(SDKConfig->SetSDKVersion(SDKVersion, SDKPath));
	SB_CHECK(SDKConfig->CreateDeviceFactory(SDKVersion, SDKPath, IID_PPV_ARGS(&DeviceFactory)));
	SB_CHECK(DeviceFactory->ApplyToGlobalState());
	SDKConfig->FreeUnusedSDKs();
	SB_LOG_INFO("Initialized DirectX SDK with version '{}'", SDKVersion);
	return S_OK;
}

HRESULT FGraphicsDevice::ChooseAdapter(const ComPtr<ID3D12DeviceFactory>& DeviceFactory,
                                       const D3D_FEATURE_LEVEL RequiredFeatureLevel,
                                       ComPtr<IDXGIAdapter4>& Adapter)
{
	for (uint32_t Index = 0; Factory->EnumAdapterByGpuPreference(
		     Index, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&Adapter)) != DXGI_ERROR_NOT_FOUND; ++Index)
	{
		DXGI_ADAPTER_DESC3 Desc;
		Adapter->GetDesc3(&Desc);
		if (!(Desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE))
		{
			ComPtr<ID3D12Device13> TempDevice;
			const HRESULT CreateResult = DeviceFactory->CreateDevice(Adapter.Get(), RequiredFeatureLevel,
			                                                         IID_PPV_ARGS(&TempDevice));
			if (FAILED(CreateResult))
				continue;
			D3D12_FEATURE_DATA_D3D12_OPTIONS12 FeatureOptions;
			const HRESULT CheckFeatureResult = TempDevice->CheckFeatureSupport(
				D3D12_FEATURE_D3D12_OPTIONS12, &FeatureOptions, sizeof(FeatureOptions));
			if (FAILED(CheckFeatureResult))
				continue;
			if (!FeatureOptions.EnhancedBarriersSupported)
				continue;
			AdapterDesc = Desc;
			break;
		}
		Adapter.Reset();
	}
	if (Adapter == nullptr)
		return DXGI_ERROR_NOT_FOUND;
	SB_LOG_INFO("DirectX Adapter Info:");
	SB_LOG_INFO("\t- Name: {}", WideStringToString(AdapterDesc.Description));
	SB_LOG_INFO("\t- Device Memory: {} MB", AdapterDesc.DedicatedVideoMemory / 1024 / 1024);
	return S_OK;
}
