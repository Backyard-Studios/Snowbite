#include "pch.h"

#include <Engine/Graphics/SwapChain.h>
#include <Engine/Graphics/GraphicsDevice.h>

FSwapChain::FSwapChain(FGraphicsDevice* InGraphicsDevice, const FSwapChainDesc& InDesc)
	: GraphicsDevice(InGraphicsDevice), Desc(InDesc)
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
	const HRESULT CreateSwapChainResult = GraphicsDevice->GetFactory()->CreateSwapChainForHwnd(
		GraphicsDevice->GetCommandQueue(), Desc.Window->GetHandle(), &SwapChainDesc, &FullscreenDesc, nullptr,
		&TempSwapChain);
	SB_D3D_ASSERT(SUCCEEDED(CreateSwapChainResult), "Failed to create swap chain!");
	const HRESULT QuerySwapChainResult = TempSwapChain->QueryInterface(IID_PPV_ARGS(&SwapChain));
	SB_D3D_ASSERT(SUCCEEDED(QuerySwapChainResult), "Failed to query swap chain!");
	TempSwapChain.Release();

	BackBuffers.resize(Desc.BufferCount);
	BackBufferDescriptors.resize(Desc.BufferCount);

	const D3D12_CPU_DESCRIPTOR_HANDLE Handle = GraphicsDevice->GetRtvDescriptorHeap()->
	                                                           GetCPUDescriptorHandleForHeapStart();
	for (uint32_t Index = 0; Index < Desc.BufferCount; ++Index)
	{
		BackBufferDescriptors[Index] = Handle;
		BackBufferDescriptors[Index].ptr += GraphicsDevice->GetRtvDescriptorSize() * Index;
	}

	QueryBackBuffers();
}

FSwapChain::~FSwapChain()
{
	ReleaseBackBuffers();
	SwapChain.Release();
}

void FSwapChain::Resize(const uint32_t InWidth, const uint32_t InHeight)
{
	ReleaseBackBuffers();
	SwapChain->ResizeBuffers(Desc.BufferCount, InWidth, InHeight, DXGI_FORMAT_UNKNOWN, SwapChainCreationFlags);
	QueryBackBuffers();
}

void FSwapChain::Present(const bool bShouldVSync)
{
	const HRESULT PresentResult = SwapChain->Present(bShouldVSync ? 1 : 0, 0);
	SB_D3D_ASSERT(PresentResult, "Failed to present swap chain!");
}

uint32_t FSwapChain::GetFrameIndex()
{
	return SwapChain->GetCurrentBackBufferIndex();
}

ComPointer<ID3D12Resource2> FSwapChain::GetBackBuffer()
{
	return BackBuffers.at(GetFrameIndex());
}

ComPointer<ID3D12Resource2> FSwapChain::GetBackBuffer(const uint32_t Index) const
{
	return BackBuffers.at(Index);
}

D3D12_CPU_DESCRIPTOR_HANDLE FSwapChain::GetBackBufferDescriptor()
{
	return BackBufferDescriptors.at(GetFrameIndex());
}

D3D12_CPU_DESCRIPTOR_HANDLE FSwapChain::GetBackBufferDescriptor(const uint32_t Index) const
{
	return BackBufferDescriptors.at(Index);
}

void FSwapChain::QueryBackBuffers()
{
	for (uint32_t Index = 0; Index < Desc.BufferCount; ++Index)
	{
		const HRESULT GetBufferResult = SwapChain->GetBuffer(Index, IID_PPV_ARGS(&BackBuffers[Index]));
		SB_D3D_ASSERT(SUCCEEDED(GetBufferResult), "Failed to get back buffer!");
		D3D12_RENDER_TARGET_VIEW_DESC RenderTargetViewDesc;
		RenderTargetViewDesc.Format = Desc.Format;
		RenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		RenderTargetViewDesc.Texture2D.MipSlice = 0;
		RenderTargetViewDesc.Texture2D.PlaneSlice = 0;
		GraphicsDevice->GetDevice()->CreateRenderTargetView(BackBuffers[Index], &RenderTargetViewDesc,
		                                                    BackBufferDescriptors[Index]);
	}
}

void FSwapChain::ReleaseBackBuffers()
{
	for (uint32_t Index = 0; Index < Desc.BufferCount; ++Index)
	{
		BackBuffers[Index].Release();
	}
}
