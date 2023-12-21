#include "pch.h"

#include <Engine/Graphics/D3D12/D3D12SwapChain.h>

#include "Engine/Graphics/D3D12/D3D12Utils.h"

FD3D12SwapChain::FD3D12SwapChain(const ComPointer<IDXGIFactory7>& InFactory, const ComPointer<ID3D12Device10>& InDevice,
                                 const ComPointer<ID3D12CommandQueue>& InCommandQueue,
                                 const FD3D12SwapChainDesc& InDesc)
	: Factory(InFactory), Device(InDevice), CommandQueue(InCommandQueue), Desc(InDesc)
{
}

FD3D12SwapChain::~FD3D12SwapChain() = default;

HRESULT FD3D12SwapChain::Initialize()
{
	DXGI_SWAP_CHAIN_DESC1 SwapChainDesc;
	SwapChainDesc.Width = Desc.Width;
	SwapChainDesc.Height = Desc.Height;
	SwapChainDesc.Format = Desc.Format;
	SwapChainDesc.Stereo = false;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SampleDesc.Quality = 0;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_BACK_BUFFER;
	SwapChainDesc.BufferCount = Desc.BufferCount;
	SwapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
	SwapChainCreationFlags = SwapChainDesc.Flags;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC FullscreenDesc;
	FullscreenDesc.Windowed = true;
	ComPointer<IDXGISwapChain1> TempSwapChain;
	const HRESULT CreateSwapChainResult = Factory->CreateSwapChainForHwnd(
		CommandQueue, Desc.Window->GetHandle(), &SwapChainDesc, &FullscreenDesc, nullptr,
		&TempSwapChain);
	SB_D3D_ASSERT_RETURN(CreateSwapChainResult, "Failed to create swap chain!");
	const HRESULT QuerySwapChainResult = TempSwapChain->QueryInterface(IID_PPV_ARGS(&SwapChain));
	SB_D3D_ASSERT_RETURN(QuerySwapChainResult, "Failed to query swap chain!");

	D3D12_DESCRIPTOR_HEAP_DESC RtvDescriptorHeapDesc;
	RtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	RtvDescriptorHeapDesc.NumDescriptors = Desc.BufferCount;
	RtvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	RtvDescriptorHeapDesc.NodeMask = 0;
	const HRESULT DescriptorHeapCreateResult = Device->CreateDescriptorHeap(
		&RtvDescriptorHeapDesc, IID_PPV_ARGS(&RtvDescriptorHeap));
	SB_D3D_ASSERT_RETURN(DescriptorHeapCreateResult, "Failed to create descriptor heap");

	BackBuffers.resize(Desc.BufferCount);
	BackBufferDescriptors.resize(Desc.BufferCount);
	const uint32_t BackBufferDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	const D3D12_CPU_DESCRIPTOR_HANDLE Handle = RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (uint32_t Index = 0; Index < Desc.BufferCount; ++Index)
	{
		BackBufferDescriptors[Index] = Handle;
		BackBufferDescriptors[Index].ptr += BackBufferDescriptorSize * Index;
	}
	const HRESULT QueryBackBuffersResult = QueryBackBuffers();
	SB_D3D_FAILED_RETURN(QueryBackBuffersResult);
	return S_OK;
}

void FD3D12SwapChain::Destroy()
{
	ReleaseBackBuffers();
	RtvDescriptorHeap.Release();
	SwapChain.Release();
}

HRESULT FD3D12SwapChain::Resize(const uint32_t InWidth, const uint32_t InHeight)
{
	ReleaseBackBuffers();
	const HRESULT ResizeBuffersResult = SwapChain->ResizeBuffers(Desc.BufferCount, InWidth, InHeight,
	                                                             DXGI_FORMAT_UNKNOWN, SwapChainCreationFlags);
	SB_D3D_ASSERT_RETURN(ResizeBuffersResult, "Failed to resize swap chain buffers!");
	const HRESULT QueryBackBuffersResult = QueryBackBuffers();
	SB_D3D_FAILED_RETURN(QueryBackBuffersResult);
	return S_OK;
}

HRESULT FD3D12SwapChain::Present(const bool bShouldVSync)
{
	const HRESULT PresentResult = SwapChain->Present(bShouldVSync ? 1 : 0,
	                                                 bShouldVSync ? 0 : DXGI_PRESENT_ALLOW_TEARING);
	SB_D3D_FAILED_RETURN(PresentResult);
	return S_OK;
}

uint32_t FD3D12SwapChain::GetFrameIndex()
{
	return SwapChain->GetCurrentBackBufferIndex();
}

ComPointer<ID3D12Resource2> FD3D12SwapChain::GetBackBuffer()
{
	return BackBuffers.at(GetFrameIndex());
}

ComPointer<ID3D12Resource2> FD3D12SwapChain::GetBackBuffer(const uint32_t Index) const
{
	return BackBuffers.at(Index);
}

D3D12_CPU_DESCRIPTOR_HANDLE FD3D12SwapChain::GetBackBufferDescriptor()
{
	return BackBufferDescriptors.at(GetFrameIndex());
}

D3D12_CPU_DESCRIPTOR_HANDLE FD3D12SwapChain::GetBackBufferDescriptor(const uint32_t Index) const
{
	return BackBufferDescriptors.at(Index);
}

HRESULT FD3D12SwapChain::QueryBackBuffers()
{
	for (uint32_t Index = 0; Index < Desc.BufferCount; ++Index)
	{
		const HRESULT GetBufferResult = SwapChain->GetBuffer(Index, IID_PPV_ARGS(&BackBuffers[Index]));
		SB_D3D_ASSERT_RETURN(GetBufferResult, "Failed to get swap chain buffer!");
		D3D12_RENDER_TARGET_VIEW_DESC RenderTargetViewDesc;
		RenderTargetViewDesc.Format = Desc.Format;
		RenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		RenderTargetViewDesc.Texture2D.MipSlice = 0;
		RenderTargetViewDesc.Texture2D.PlaneSlice = 0;
		Device->CreateRenderTargetView(BackBuffers[Index], &RenderTargetViewDesc,
		                               BackBufferDescriptors[Index]);
	}
	return S_OK;
}

void FD3D12SwapChain::ReleaseBackBuffers()
{
	for (uint32_t Index = 0; Index < Desc.BufferCount; ++Index)
	{
		BackBuffers[Index].Release();
	}
}
