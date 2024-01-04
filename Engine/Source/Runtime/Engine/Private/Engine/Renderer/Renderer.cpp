#include "pch.h"

#include <Engine/Renderer/Renderer.h>

#define SB_D3D12_SDK_VERSION 611  // NOLINT(modernize-macro-to-enum)
#define SB_D3D12_SDK_PATH TEXT(".\\D3D12\\")

std::shared_ptr<FWindow> FRenderer::Window = nullptr;
#ifdef SB_DEBUG
ComPtr<IDXGIDebug1> FRenderer::DXGIDebug = nullptr;
#endif
ComPtr<IDXGIFactory7> FRenderer::Factory = nullptr;
std::shared_ptr<FD3D12Device> FRenderer::Device = nullptr;
std::shared_ptr<FD3D12CommandQueue> FRenderer::CommandQueue = nullptr;
std::shared_ptr<FSwapChain> FRenderer::SwapChain = nullptr;
std::array<FFrameContext, FRenderer::BufferCount> FRenderer::FrameContexts;
uint32_t FRenderer::CurrentFrameIndex = 0;
bool FRenderer::bIsResizing = false;

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
	Device = std::make_shared<FD3D12Device>(Factory);
	SB_CHECK(Device->Initialize(TempAdapter, DeviceFactory));
	DeviceFactory.Reset();
	SDKConfig->FreeUnusedSDKs();
	SDKConfig.Reset();

	D3D12_FEATURE_DATA_D3D12_OPTIONS12 FeatureOptions;
	SB_CHECK(
		Device->GetD3D12Device()->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS12, &FeatureOptions, sizeof(
			FeatureOptions)));
	if (!FeatureOptions.EnhancedBarriersSupported)
		FPlatform::Fatal("Enhanced barriers not supported");

	CommandQueue = std::make_shared<FD3D12CommandQueue>(Device);
	SB_CHECK(CommandQueue->Initialize(D3D12_COMMAND_LIST_TYPE_DIRECT));

	FSwapChainDesc SwapChainDesc;
	SwapChainDesc.BufferCount = BufferCount;
	SwapChainDesc.Factory = Factory;
	SwapChainDesc.Device = Device->GetD3D12Device();
	SwapChainDesc.CommandQueue = CommandQueue->GetD3D12CommandQueue();
	SwapChainDesc.Window = Window;
	SwapChain = std::make_shared<FSwapChain>();
	SB_CHECK(SwapChain->Initialize(SwapChainDesc));

	for (uint32_t Index = 0; Index < BufferCount; ++Index)
	{
		SB_CHECK(
			Device->GetD3D12Device()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&
				FrameContexts[Index].
				CommandAllocator)));
		SB_CHECK(FrameContexts[Index].CommandAllocator->Reset());
		SB_CHECK(
			Device->GetD3D12Device()->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
				D3D12_COMMAND_LIST_FLAG_NONE,
				IID_PPV_ARGS(&FrameContexts[Index].CommandList)));
		FrameContexts[Index].TempFence = std::make_shared<FD3D12Fence>(Device);
		SB_CHECK(FrameContexts[Index].TempFence->Initialize());
	}
	return S_OK;
}

void FRenderer::Shutdown()
{
	const HRESULT FlushResult = FlushFrames();
	if (FAILED(FlushResult))
	{
		FPlatform::PrintHRESULT(FlushResult);
		std::cout << "Failed to flush frames" << std::endl;
	}
	for (FFrameContext& FrameContext : FrameContexts)
	{
		FrameContext.CommandAllocator.Reset();
		FrameContext.CommandList.Reset();
		FrameContext.TempFence.reset();
	}
	SB_SAFE_DESTROY(SwapChain);
	SB_SAFE_DESTROY(CommandQueue);
	SB_SAFE_DESTROY(Device);
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
	bIsResizing = true;
	SB_CHECK(FlushFrames());
	SB_CHECK(SwapChain->Resize(Width, Height));
	bIsResizing = false;
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
	if (bIsResizing)
		return S_OK;
	CurrentFrameIndex = SwapChain->GetBackBufferIndex();
	const FFrameContext& FrameContext = FrameContexts[CurrentFrameIndex];

	if (!FrameContext.TempFence->IsCompleted())
	{
		SB_CHECK(FrameContext.TempFence->Wait());
	}
	FrameContext.TempFence->IncrementValue();

	const ComPtr<ID3D12GraphicsCommandList9> CommandList = FrameContext.CommandList;
	SB_CHECK(FrameContext.CommandAllocator->Reset());
	SB_CHECK(CommandList->Reset(FrameContext.CommandAllocator.Get(), nullptr));

	SwapChain->TransitionBackBuffersToRenderTarget(CommandList);

	const D3D12_CPU_DESCRIPTOR_HANDLE RTVHandle = SwapChain->GetCurrentBackBufferHandle();
	CommandList->OMSetRenderTargets(1, &RTVHandle, false, nullptr);
	constexpr float ClearColor[] = {0.08f, 0.08f, 0.08f, 1.0f};
	CommandList->ClearRenderTargetView(RTVHandle, ClearColor, 0, nullptr);
	return S_OK;
}

HRESULT FRenderer::EndFrame()
{
	if (bIsResizing)
		return S_OK;
	const FFrameContext& FrameContext = FrameContexts[CurrentFrameIndex];
	const ComPtr<ID3D12GraphicsCommandList9> CommandList = FrameContext.CommandList;

	SwapChain->TransitionBackBuffersToPresent(CommandList);

	SB_CHECK(CommandList->Close());

	ID3D12CommandList* CommandLists[] = {CommandList.Get()};
	CommandQueue->ExecuteCommandLists(1, CommandLists);

	SB_CHECK(SwapChain->Present(true));

	SB_CHECK(CommandQueue->Signal(FrameContext.TempFence));
	// Not waiting for the fence to complete, because we're going to wait for it when the frame is used again.
	// This is better for performance, because we don't have to wait for the GPU to finish the current frame which
	// will not be used again for 2 iterations. While this frame executes, we can start recording and executing the next frame.
	return S_OK;
}

HRESULT FRenderer::SignalAndWaitForFence(const std::shared_ptr<FD3D12Fence>& Fence)
{
	SB_CHECK(CommandQueue->Signal(Fence));
	SB_CHECK(Fence->Wait());
	return S_OK;
}

HRESULT FRenderer::WaitForFrame(const uint32_t Index)
{
	return SignalAndWaitForFence(FrameContexts[Index].TempFence);
}

HRESULT FRenderer::FlushFrames()
{
	for (uint32_t Index = 0; Index < BufferCount; ++Index)
		SB_CHECK(WaitForFrame(Index));
	return S_OK;
}
