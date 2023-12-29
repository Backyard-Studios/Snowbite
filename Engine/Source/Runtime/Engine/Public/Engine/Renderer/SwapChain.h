#pragma once

#include <Engine/Core/Definitions.h>

#include <vector>

struct SNOWBITE_API FSwapChainDesc
{
	uint32_t BufferCount = 3;
	ComPtr<IDXGIFactory7> Factory;
	ComPtr<ID3D12Device13> Device;
	ComPtr<ID3D12CommandQueue> CommandQueue;
	std::shared_ptr<FWindow> Window;
};

class SNOWBITE_API FSwapChain
{
public:
	FSwapChain();
	~FSwapChain();

	[[nodiscard]] HRESULT Initialize(const FSwapChainDesc& InDesc);
	void Destroy();

	[[nodiscard]] HRESULT Resize(uint32_t Width, uint32_t Height);
	void TransitionBackBuffersToPresent(const ComPtr<ID3D12GraphicsCommandList9>& CommandList) const;
	void TransitionBackBuffersToRenderTarget(const ComPtr<ID3D12GraphicsCommandList9>& CommandList) const;
	[[nodiscard]] HRESULT Present(bool bShouldVSync) const;

	[[nodiscard]] uint32_t GetBackBufferCount() const;
	[[nodiscard]] uint32_t GetBackBufferIndex() const;
	[[nodiscard]] ComPtr<ID3D12Resource2> GetCurrentBackBuffer() const;
	[[nodiscard]] ComPtr<ID3D12Resource2> GetBackBuffer(uint32_t Index) const;
	[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferHandle() const;
	[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetBackBufferHandle(uint32_t Index) const;

private:
	HRESULT QueryBackBuffers();
	void DestroyBackBuffers();

private:
	FSwapChainDesc Desc;

	uint32_t SwapChainFlags = 0;
	ComPtr<IDXGISwapChain4> SwapChain;
	ComPtr<ID3D12DescriptorHeap> RTVHeap;
	std::vector<ComPtr<ID3D12Resource2>> BackBuffers;
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> BackBufferHandles;
};
