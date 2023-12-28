#include "pch.h"

#include <Engine/Renderer/Renderer.h>

#define SB_D3D12_SDK_VERSION 611  // NOLINT(modernize-macro-to-enum)
#define SB_D3D12_SDK_PATH TEXT(".\\D3D12\\")

std::shared_ptr<FWindow> FRenderer::Window = nullptr;
#ifdef SB_DEBUG
ComPtr<IDXGIDebug1> FRenderer::DXGIDebug = nullptr;
#endif
ComPtr<IDXGIFactory7> FRenderer::Factory = nullptr;
ComPtr<ID3D12Device13> FRenderer::Device = nullptr;
ComPtr<ID3D12CommandQueue> FRenderer::CommandQueue = nullptr;
std::shared_ptr<FSwapChain> FRenderer::SwapChain = nullptr;
std::array<FFrameContext, FRenderer::BufferCount> FRenderer::FrameContexts;
uint32_t FRenderer::CurrentFrameIndex = 0;

HRESULT FRenderer::Initialize(const std::shared_ptr<FWindow>& InWindow)
{
	Window = InWindow;

	ComPtr<ID3D12SDKConfiguration1> SDKConfig;
	ComPtr<ID3D12DeviceFactory> DeviceFactory;
	SB_CHECK(InitializeSDK(SDKConfig, DeviceFactory));

	InitializeDebugLayer();

	SB_CHECK(InitializeFactory());

	ComPtr<IDXGIAdapter4> TempAdapter;
	SB_CHECK(ChooseAdapter(DeviceFactory, TempAdapter));
	SB_CHECK(DeviceFactory->CreateDevice(TempAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&Device)));
	DeviceFactory.Reset();
	SDKConfig->FreeUnusedSDKs();
	SDKConfig.Reset();

	D3D12_FEATURE_DATA_D3D12_OPTIONS12 FeatureOptions;
	SB_CHECK(Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS12, &FeatureOptions, sizeof(FeatureOptions)));
	if (!FeatureOptions.EnhancedBarriersSupported)
		FPlatform::Fatal("Enhanced barriers not supported");

	D3D12_COMMAND_QUEUE_DESC CommandQueueDesc;
	CommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	CommandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
	CommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	CommandQueueDesc.NodeMask = 0;
	SB_CHECK(Device->CreateCommandQueue(&CommandQueueDesc, IID_PPV_ARGS(&CommandQueue)));

	FSwapChainDesc SwapChainDesc;
	SwapChainDesc.BufferCount = BufferCount;
	SwapChainDesc.Factory = Factory;
	SwapChainDesc.Device = Device;
	SwapChainDesc.CommandQueue = CommandQueue;
	SwapChainDesc.Window = Window;
	SwapChain = std::make_shared<FSwapChain>();
	SB_CHECK(SwapChain->Initialize(SwapChainDesc));

	for (uint32_t Index = 0; Index < BufferCount; ++Index)
	{
		SB_CHECK(
			Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&FrameContexts[Index].
				CommandAllocator)));
		SB_CHECK(Device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE,
			IID_PPV_ARGS(&FrameContexts[Index].CommandList)));
		SB_CHECK(FrameContexts[Index].CommandList->Close());
		SB_CHECK(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&FrameContexts[Index].Fence)));
		FrameContexts[Index].FenceValue = 0;
		FrameContexts[Index].FenceEvent = CreateEvent(nullptr, false, false, nullptr);
	}
	return S_OK;
}

void FRenderer::Shutdown()
{
	const HRESULT FlushResult = FlushFrames();
	if (FAILED(FlushResult))
		std::cout << "Failed to flush frames" << std::endl;
	SB_SAFE_DESTROY(SwapChain);
	CommandQueue.Reset();
	Device.Reset();
	Factory.Reset();
#ifdef SB_DEBUG
	if (DXGIDebug)
	{
		OutputDebugString(TEXT("Reporting live dxgi objects:\n"));
		DXGIDebug->ReportLiveObjects(DXGI_DEBUG_ALL,
		                             static_cast<DXGI_DEBUG_RLO_FLAGS>(DXGI_DEBUG_RLO_DETAIL |
			                             DXGI_DEBUG_RLO_IGNORE_INTERNAL));
	}
	DXGIDebug.Reset();
#endif
}

HRESULT FRenderer::Resize(const uint32_t Width, const uint32_t Height)
{
	SB_CHECK(FlushFrames());
	SB_CHECK(SwapChain->Resize(Width, Height));
	return S_OK;
}

#ifdef SB_DEBUG
void FRenderer::InitializeDebugLayer()
{
	const HRESULT GetDXGIDebugResult = DXGIGetDebugInterface1(0, IID_PPV_ARGS(&DXGIDebug));
	if (SUCCEEDED(GetDXGIDebugResult))
	{
		DXGIDebug->EnableLeakTrackingForThread();
	}
	ComPtr<ID3D12Debug6> D3D12Debug;
	const HRESULT GetD3D12DebugResult = D3D12GetDebugInterface(IID_PPV_ARGS(&D3D12Debug));
	if (SUCCEEDED(GetD3D12DebugResult))
	{
		D3D12Debug->EnableDebugLayer();
		D3D12Debug->SetEnableGPUBasedValidation(true);
		D3D12Debug->SetEnableSynchronizedCommandQueueValidation(true);
	}
}
#endif

HRESULT FRenderer::InitializeFactory()
{
#ifdef SB_DEBUG
	constexpr uint32_t FactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#else
	constexpr uint32_t FactoryFlags = 0;
#endif
	SB_CHECK(CreateDXGIFactory2(FactoryFlags, IID_PPV_ARGS(&Factory)));
	return S_OK;
}

HRESULT FRenderer::ChooseAdapter(const ComPtr<ID3D12DeviceFactory>& DeviceFactory, ComPtr<IDXGIAdapter4>& TempAdapter)
{
	for (int Index = 0; Index < Factory->EnumAdapterByGpuPreference(Index, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
	                                                                IID_PPV_ARGS(&TempAdapter)); ++Index)
	{
		DXGI_ADAPTER_DESC3 Desc;
		TempAdapter->GetDesc3(&Desc);
		if (!(Desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE))
		{
			if (SUCCEEDED(
				DeviceFactory->CreateDevice(TempAdapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device13), nullptr
				)))
			{
				break;
			}
		}
		TempAdapter.Reset();
	}
	if (TempAdapter == nullptr)
		return DXGI_ERROR_NOT_FOUND;
	return S_OK;
}

HRESULT FRenderer::InitializeSDK(ComPtr<ID3D12SDKConfiguration1>& SDKConfig, ComPtr<ID3D12DeviceFactory>& DeviceFactory)
{
	SB_CHECK(D3D12GetInterface(CLSID_D3D12SDKConfiguration, IID_PPV_ARGS(&SDKConfig)));
	SB_CHECK(SDKConfig->SetSDKVersion(SB_D3D12_SDK_VERSION, SB_D3D12_SDK_PATH));
	SB_CHECK(SDKConfig->CreateDeviceFactory(SB_D3D12_SDK_VERSION, SB_D3D12_SDK_PATH, IID_PPV_ARGS(&DeviceFactory)));
	return S_OK;
}

HRESULT FRenderer::BeginFrame()
{
	const FFrameContext& FrameContext = FrameContexts[CurrentFrameIndex];
	SB_CHECK(FrameContext.CommandAllocator->Reset());
	SB_CHECK(FrameContext.CommandList->Reset(FrameContext.CommandAllocator.Get(), nullptr));
	return S_OK;
}

HRESULT FRenderer::EndFrame()
{
	const FFrameContext& FrameContext = FrameContexts[CurrentFrameIndex];
	SB_CHECK(FrameContext.CommandList->Close());

	ID3D12CommandList* CommandLists[] = {FrameContext.CommandList.Get()};
	CommandQueue->ExecuteCommandLists(1, CommandLists);

	SB_CHECK(SwapChain->Present(true));
	return S_OK;
}

HRESULT FRenderer::WaitForFence(const ComPtr<ID3D12Fence1>& Fence, uint64_t FenceValue, const HANDLE FenceEvent)
{
	FenceValue++;
	SB_CHECK(Fence->SetEventOnCompletion(FenceValue, FenceEvent));
	SB_CHECK(CommandQueue->Signal(Fence.Get(), FenceValue));
	const DWORD WaitResult = WaitForSingleObject(FenceEvent, 20000);
	if (WaitResult != WAIT_OBJECT_0)
		return DXGI_ERROR_WAIT_TIMEOUT;
	return S_OK;
}

HRESULT FRenderer::WaitForFrame(const uint32_t Index)
{
	const FFrameContext& FrameContext = FrameContexts[Index];
	return WaitForFence(FrameContext.Fence, FrameContext.FenceValue, FrameContext.FenceEvent);
}

HRESULT FRenderer::FlushFrames()
{
	for (uint32_t Index = 0; Index < BufferCount; ++Index)
		SB_CHECK(WaitForFrame(Index));
	return S_OK;
}
