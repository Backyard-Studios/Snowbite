#include "pch.h"

#include <Engine/Graphics/D3D12/D3D12SwapChain.h>

#include "Engine/Graphics/D3D12/D3D12CommandQueue.h"
#include "Engine/Graphics/D3D12/D3D12GraphicsDevice.h"
#include "Engine/Graphics/D3D12/D3D12GraphicsInterface.h"

FD3D12SwapChain::FD3D12SwapChain()
{
}

FD3D12SwapChain::~FD3D12SwapChain()
{
}

HRESULT FD3D12SwapChain::Initialize(const std::shared_ptr<IGraphicsInterface> InGraphicsInterface,
                                    const HWND WindowHandle)
{
	const FD3D12GraphicsInterface* GraphicsInterface = InGraphicsInterface->As<FD3D12GraphicsInterface>();
	const FD3D12GraphicsDevice* GraphicsDevice = GraphicsInterface->GetDevice()->As<FD3D12GraphicsDevice>();
	Device = GraphicsDevice->GetNativeDevice();
	const ComPtr<IDXGIFactory7> Factory = GraphicsInterface->GetFactory();
	const ComPtr<ID3D12CommandQueue> CommandQueue = GraphicsInterface->GetDevice()->GetContext().CommandQueue->As<
		FD3D12CommandQueue>()->GetNativeCommandQueue();
	const std::shared_ptr<FWindow> Window = FWindowManager::GetWindow(WindowHandle);

	BackBufferCount = GraphicsInterface->GetDevice()->GetContext().BufferCount;
	BackBufferSize = Window->GetSize();

	DXGI_SWAP_CHAIN_DESC1 SwapChainDesc;
	SwapChainDesc.Width = BackBufferSize.X;
	SwapChainDesc.Height = BackBufferSize.Y;
	SwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChainDesc.Stereo = FALSE;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SampleDesc.Quality = 0;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.BufferCount = BackBufferCount;
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
	const HRESULT CreateResult = Factory->CreateSwapChainForHwnd(CommandQueue.Get(), WindowHandle, &SwapChainDesc,
	                                                             &FullscreenDesc,
	                                                             nullptr, &TempSwapChain);
	SB_CHECK_RESULT_LOW(CreateResult);
	const HRESULT QueryInterfaceResult = TempSwapChain->QueryInterface(IID_PPV_ARGS(&SwapChain));
	SB_CHECK_RESULT_LOW(QueryInterfaceResult);

	D3D12_DESCRIPTOR_HEAP_DESC BackBufferDescriptorHeapDesc;
	BackBufferDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	BackBufferDescriptorHeapDesc.NumDescriptors = BackBufferCount;
	BackBufferDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	BackBufferDescriptorHeapDesc.NodeMask = 0;
	const HRESULT CreateHeapResult = Device->CreateDescriptorHeap(&BackBufferDescriptorHeapDesc,
	                                                              IID_PPV_ARGS(&BackBufferDescriptorHeap));
	SB_CHECK_RESULT_LOW(CreateHeapResult);

	const uint32_t DescriptorHeapSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	for (uint32_t Index = 0; Index < BackBufferCount; ++Index)
	{
		BackBufferHandles[Index] = BackBufferDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		BackBufferHandles[Index].ptr += DescriptorHeapSize * Index;
	}

	const HRESULT QueryBackBuffersResult = QueryBackBuffers();
	SB_CHECK_RESULT(QueryBackBuffersResult);
	return S_OK;
}

void FD3D12SwapChain::Destroy()
{
	DestroyBackBuffers();
	SwapChain.Reset();
}

HRESULT FD3D12SwapChain::Present(const bool bShouldVSync)
{
	const uint32_t SyncInterval = bShouldVSync ? 1 : 0;
	const uint32_t PresentFlags = bShouldVSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
	const HRESULT PresentResult = SwapChain->Present(SyncInterval, PresentFlags);
	SB_CHECK_RESULT_LOW(PresentResult);
	return S_OK;
}

void FD3D12SwapChain::Resize(const FUInt2& InSize)
{
	BackBufferSize = InSize;
	DestroyBackBuffers();
	const HRESULT ResizeResult = SwapChain->ResizeBuffers(BackBufferCount, InSize.X, InSize.Y, DXGI_FORMAT_UNKNOWN,
	                                                      SwapChainFlags);
	SB_CHECK_RESULT_FATAL(ResizeResult);
	const HRESULT QueryBackBuffersResult = QueryBackBuffers();
	SB_CHECK_RESULT_FATAL(QueryBackBuffersResult);
}

uint32_t FD3D12SwapChain::GetBackBufferIndex()
{
	return SwapChain->GetCurrentBackBufferIndex();
}

HRESULT FD3D12SwapChain::QueryBackBuffers()
{
	for (uint32_t Index = 0; Index < BackBufferCount; ++Index)
	{
		const HRESULT GetResult = SwapChain->GetBuffer(Index, IID_PPV_ARGS(&BackBuffers[Index]));
		SB_CHECK_RESULT_LOW(GetResult);
		D3D12_RENDER_TARGET_VIEW_DESC RenderTargetViewDesc;
		RenderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		RenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		RenderTargetViewDesc.Texture2D.MipSlice = 0;
		RenderTargetViewDesc.Texture2D.PlaneSlice = 0;
		Device->CreateRenderTargetView(BackBuffers[Index].Get(), &RenderTargetViewDesc, BackBufferHandles[Index]);
	}
	return S_OK;
}

void FD3D12SwapChain::DestroyBackBuffers()
{
	BackBufferHandles.clear();
	for (const ComPtr<ID3D12Resource2>& BackBuffer : BackBuffers)
		BackBuffer->Release();
	BackBuffers.clear();
}
