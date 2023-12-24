#include "pch.h"

#include <Engine/Graphics/D3D12/D3D12GraphicsInterface.h>


FD3D12GraphicsInterface::FD3D12GraphicsInterface()
{
}

FD3D12GraphicsInterface::~FD3D12GraphicsInterface()
{
}

HRESULT FD3D12GraphicsInterface::Initialize()
{
#if SB_D3D12_ENABLE_DEBUG_LAYER
	{
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&DXGIDebug))))
			DXGIDebug->EnableLeakTrackingForThread();
		ComPtr<ID3D12Debug6> D3D12Debug;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&D3D12Debug))))
		{
			D3D12Debug->EnableDebugLayer();
			D3D12Debug->SetEnableGPUBasedValidation(true);
		}
		D3D12Debug.Reset();
	}
#endif

#if SB_D3D12_ENABLE_DEBUG_LAYER
	uint32_t FactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#else
	uint32_t FactoryFlags = 0;
#endif
	const HRESULT Result = CreateDXGIFactory2(FactoryFlags, IID_PPV_ARGS(&Factory));
	SB_CHECK_RESULT(Result, TEXT("Failed to create DXGI factory"));
	return S_OK;
}

void FD3D12GraphicsInterface::Destroy()
{
	Factory.Reset();
#if SB_D3D12_ENABLE_DEBUG_LAYER
	if (DXGIDebug)
	{
		OutputDebugString(TEXT("Live DXGI objects:\n"));
		DXGIDebug->ReportLiveObjects(DXGI_DEBUG_ALL,
		                             static_cast<DXGI_DEBUG_RLO_FLAGS>(DXGI_DEBUG_RLO_DETAIL |
			                             DXGI_DEBUG_RLO_IGNORE_INTERNAL));
		DXGIDebug.Reset();
	}
#endif
}
