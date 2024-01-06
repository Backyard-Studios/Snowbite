#include "pch.h"

#include <Engine/Renderer/SwapChain.h>

FSwapChain::FSwapChain()
{
}

FSwapChain::~FSwapChain()
{
}

HRESULT FSwapChain::Initialize(const FSwapChainDesc& InDesc)
{
	Desc = InDesc;

	DXGI_SWAP_CHAIN_DESC1 SwapChainDesc;
	SwapChainDesc.Width = Desc.Window->GetSize().X;
	SwapChainDesc.Height = Desc.Window->GetSize().Y;
	SwapChainDesc.Stereo = FALSE;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SampleDesc.Quality = 0;
	SwapChainDesc.Format = Desc.Format;
	SwapChainDesc.BufferCount = Desc.BufferCount;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_BACK_BUFFER;
	SwapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
	SwapChainFlags = SwapChainDesc.Flags;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC FullscreenDesc;
	FullscreenDesc.RefreshRate.Numerator = 60;
	FullscreenDesc.RefreshRate.Denominator = 1;
	FullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	FullscreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	FullscreenDesc.Windowed = TRUE;

	ComPtr<IDXGISwapChain1> TempSwapChain;
	SB_CHECK(
		Desc.Factory->CreateSwapChainForHwnd(Desc.CommandQueue.Get(), Desc.Window->GetNativeHandle(), &SwapChainDesc, &
			FullscreenDesc, nullptr, &TempSwapChain));
	SB_CHECK(TempSwapChain.As(&SwapChain));

	BackBuffers.resize(Desc.BufferCount);
	BackBufferHandles.resize(Desc.BufferCount);
	const uint32_t DescriptorHeapSize = Desc.Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	for (uint32_t Index = 0; Index < Desc.BufferCount; ++Index)
	{
		BackBufferHandles[Index] = Desc.RTVHeap->GetCPUDescriptorHandleForHeapStart();
		BackBufferHandles[Index].ptr += DescriptorHeapSize * Index;
	}

	SB_CHECK(QueryBackBuffers());
	return S_OK;
}

void FSwapChain::Destroy()
{
	DestroyBackBuffers();
	SB_SAFE_COM_RESET(SwapChain);
}

uint32_t FSwapChain::GetBackBufferIndex() const
{
	return SwapChain->GetCurrentBackBufferIndex();
}

HRESULT FSwapChain::Resize(const uint32_t NewWidth, const uint32_t NewHeight)
{
	Width = NewWidth;
	Height = NewHeight;
	DestroyBackBuffers();
	SB_CHECK(SwapChain->ResizeBuffers(Desc.BufferCount, Width, Height, DXGI_FORMAT_UNKNOWN, SwapChainFlags));
	SB_CHECK(QueryBackBuffers());
	return S_OK;
}

HRESULT FSwapChain::Present(const bool bShouldVSync) const
{
	const uint32_t SyncInterval = bShouldVSync ? 1 : 0;
	const uint32_t PresentFlags = bShouldVSync ? 0 : DXGI_PRESENT_ALLOW_TEARING;
	SB_CHECK(SwapChain->Present(SyncInterval, PresentFlags));
	return S_OK;
}

ComPtr<ID3D12Resource2> FSwapChain::GetBackBuffer(const uint32_t Index)
{
	return BackBuffers.at(Index);
}

D3D12_CPU_DESCRIPTOR_HANDLE FSwapChain::GetBackBufferHandle(const uint32_t Index) const
{
	return BackBufferHandles.at(Index);
}

HRESULT FSwapChain::QueryBackBuffers()
{
	for (uint32_t Index = 0; Index < Desc.BufferCount; ++Index)
	{
		SB_CHECK(SwapChain->GetBuffer(Index, IID_PPV_ARGS(&BackBuffers[Index])));
		D3D12_RENDER_TARGET_VIEW_DESC RenderTargetViewDesc;
		RenderTargetViewDesc.Format = Desc.Format;
		RenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		RenderTargetViewDesc.Texture2D.MipSlice = 0;
		RenderTargetViewDesc.Texture2D.PlaneSlice = 0;
		Desc.Device->CreateRenderTargetView(BackBuffers[Index].Get(), &RenderTargetViewDesc, BackBufferHandles[Index]);
	}
	return S_OK;
}

void FSwapChain::DestroyBackBuffers()
{
	for (ComPtr<ID3D12Resource2>& BackBuffer : BackBuffers)
		BackBuffer.Reset();
	BackBuffers.clear();
	BackBuffers.resize(Desc.BufferCount);
}
