#pragma once

#include <Engine/Core/Definitions.h>

#include "D3D12Device.h"
#include "Engine/Graphics/Window.h"

class FGraphicsDevice;

struct SNOWBITE_API FD3D12SwapChainDesc
{
	uint32_t Width = 0;
	uint32_t Height = 0;
	uint32_t BufferCount = 0;
	DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	std::shared_ptr<FWindow> Window;
};

class SNOWBITE_API FD3D12SwapChain
{
public:
	FD3D12SwapChain(const ComPointer<IDXGIFactory7>& InFactory, const ComPointer<ID3D12Device10>& InDevice,
	                const ComPointer<ID3D12CommandQueue>& InCommandQueue, const FD3D12SwapChainDesc& InDesc);
	~FD3D12SwapChain();

	HRESULT Initialize();
	void Destroy();

	HRESULT Resize(uint32_t InWidth, uint32_t InHeight);
	HRESULT Present(bool bShouldVSync = true);

	[[nodiscard]] uint32_t GetFrameIndex();
	[[nodiscard]] ComPointer<ID3D12Resource2> GetBackBuffer();
	[[nodiscard]] ComPointer<ID3D12Resource2> GetBackBuffer(uint32_t Index) const;
	[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetBackBufferDescriptor();
	[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetBackBufferDescriptor(uint32_t Index) const;

private:
	HRESULT QueryBackBuffers();
	void ReleaseBackBuffers();

private:
	ComPointer<IDXGIFactory7> Factory;
	ComPointer<ID3D12Device10> Device;
	ComPointer<ID3D12CommandQueue> CommandQueue;
	FD3D12SwapChainDesc Desc;

	uint32_t SwapChainCreationFlags = 0;
	ComPointer<IDXGISwapChain4> SwapChain;
	ComPointer<ID3D12DescriptorHeap> RtvDescriptorHeap;
	std::vector<ComPointer<ID3D12Resource2>> BackBuffers;
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> BackBufferDescriptors;
};
