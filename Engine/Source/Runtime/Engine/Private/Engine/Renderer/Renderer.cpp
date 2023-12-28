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

HRESULT FRenderer::Initialize(const std::shared_ptr<FWindow>& InWindow)
{
	Window = InWindow;

	ComPtr<ID3D12SDKConfiguration1> SDKConfig;
	ComPtr<ID3D12DeviceFactory> DeviceFactory;
	const HRESULT InitializeSDKResult = InitializeSDK(SDKConfig, DeviceFactory);
	SB_CHECK_RESULT(InitializeSDKResult);

	InitializeDebugLayer();

	const HRESULT InitializeFactoryResult = InitializeFactory();
	SB_CHECK_RESULT(InitializeFactoryResult);

	ComPtr<IDXGIAdapter4> TempAdapter;
	const HRESULT ChooseAdapterResult = ChooseAdapter(DeviceFactory, TempAdapter);
	SB_CHECK_RESULT_LOW(ChooseAdapterResult);
	const HRESULT DeviceCreateResult = DeviceFactory->CreateDevice(TempAdapter.Get(), D3D_FEATURE_LEVEL_12_0,
	                                                               IID_PPV_ARGS(&Device));
	SB_CHECK_RESULT_LOW(DeviceCreateResult);
	DeviceFactory.Reset();
	SDKConfig->FreeUnusedSDKs();
	SDKConfig.Reset();

	D3D12_FEATURE_DATA_D3D12_OPTIONS12 FeatureOptions;
	const HRESULT CheckFeatureResult = Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS12, &FeatureOptions,
	                                                               sizeof(FeatureOptions));
	SB_CHECK_RESULT_LOW(CheckFeatureResult);
	if (!FeatureOptions.EnhancedBarriersSupported)
		FPlatform::Fatal("Enhanced barriers not supported");
	D3D12_COMMAND_QUEUE_DESC CommandQueueDesc;
	CommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	CommandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
	CommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	CommandQueueDesc.NodeMask = 0;
	const HRESULT QueueCreateResult = Device->CreateCommandQueue(&CommandQueueDesc, IID_PPV_ARGS(&CommandQueue));
	SB_CHECK_RESULT_LOW(QueueCreateResult);
	return S_OK;
}

void FRenderer::Shutdown()
{
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
	const HRESULT CreateFactoryResult = CreateDXGIFactory2(FactoryFlags, IID_PPV_ARGS(&Factory));
	SB_CHECK_RESULT_LOW(CreateFactoryResult);
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
	const HRESULT GetConfigResult = D3D12GetInterface(CLSID_D3D12SDKConfiguration, IID_PPV_ARGS(&SDKConfig));
	SB_CHECK_RESULT_LOW(GetConfigResult);
	const HRESULT SetVersionResult = SDKConfig->SetSDKVersion(SB_D3D12_SDK_VERSION, SB_D3D12_SDK_PATH);
	SB_CHECK_RESULT_LOW(SetVersionResult);
	const HRESULT CreateFactoryResult = SDKConfig->CreateDeviceFactory(SB_D3D12_SDK_VERSION, SB_D3D12_SDK_PATH,
	                                                                   IID_PPV_ARGS(&DeviceFactory));
	SB_CHECK_RESULT_LOW(CreateFactoryResult);
	return S_OK;
}

void FRenderer::BeginFrame()
{
}

void FRenderer::EndFrame()
{
}
