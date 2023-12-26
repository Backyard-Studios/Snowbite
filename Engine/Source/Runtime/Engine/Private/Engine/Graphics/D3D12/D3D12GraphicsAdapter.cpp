#include "pch.h"

#include <Engine/Graphics/D3D12/D3D12GraphicsAdapter.h>

FD3D12GraphicsAdapter::FD3D12GraphicsAdapter(const ComPtr<IDXGIAdapter4>& InAdapter)
	: Adapter(InAdapter)
{
	const HRESULT GetDescResult = InAdapter->GetDesc3(&Desc);
	SB_CHECK_RESULT_FATAL(GetDescResult, "Failed to get DXGI adapter description");
	ComPtr<ID3D12Device13> TempDevice;
	const HRESULT CreateDeviceResult = D3D12CreateDevice(InAdapter.Get(), D3D_FEATURE_LEVEL_11_0,
	                                                     IID_PPV_ARGS(&TempDevice));
	if (SUCCEEDED(CreateDeviceResult))
	{
		constexpr D3D_FEATURE_LEVEL FeatureLevel = D3D_FEATURE_LEVEL_12_2;
		D3D12_FEATURE_DATA_FEATURE_LEVELS FeatureLevels = {};
		FeatureLevels.NumFeatureLevels = 1;
		FeatureLevels.pFeatureLevelsRequested = &FeatureLevel;
		const HRESULT GetFeatureLevelResult = TempDevice->CheckFeatureSupport(
			D3D12_FEATURE_FEATURE_LEVELS, &FeatureLevels, sizeof(FeatureLevels));
		if (SUCCEEDED(GetFeatureLevelResult))
		{
			MaxSupportedFeatureLevel = FeatureLevels.MaxSupportedFeatureLevel;
		}
		TempDevice.Reset();
	}
}

FD3D12GraphicsAdapter::~FD3D12GraphicsAdapter()
{
}

void FD3D12GraphicsAdapter::Destroy()
{
	Adapter.Reset();
}
