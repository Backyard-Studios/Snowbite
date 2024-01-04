#include "pch.h"

#include <Engine/Renderer/D3D12Device.h>

FD3D12Device::FD3D12Device(const ComPtr<IDXGIFactory7>& InFactory)
	: Factory(InFactory)
{
}

FD3D12Device::~FD3D12Device()
{
}

HRESULT FD3D12Device::Initialize(const ComPtr<IDXGIAdapter4>& Adapter, const ComPtr<ID3D12DeviceFactory>& DeviceFactory)
{
	SB_CHECK(DeviceFactory->CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&Device)));
	return S_OK;
}

void FD3D12Device::Destroy()
{
	Device.Reset();
	Factory.Reset();
}
