#pragma once

#include <Engine/Core/Definitions.h>

struct SNOWBITE_API FSwapChainDesc
{
	ComPtr<IDXGIFactory7> Factory;
	ComPtr<ID3D12Device13> Device;
	ComPtr<ID3D12CommandQueue> CommandQueue;
	ComPtr<ID3D12DescriptorHeap> RTVHeap;
	std::shared_ptr<FWindow> Window;
	uint32_t BufferCount = 3;
	DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM;
};

class SNOWBITE_API FSwapChain
{
public:
	FSwapChain();
	~FSwapChain();

	[[nodiscard]] HRESULT Initialize(const FSwapChainDesc& InDesc);
	void Destroy();

	[[nodiscard]] uint32_t GetBackBufferIndex() const;

	[[nodiscard]] HRESULT Resize(uint32_t NewWidth, uint32_t NewHeight);
	[[nodiscard]] HRESULT Present(bool bShouldVSync) const;

	[[nodiscard]] ComPtr<ID3D12Resource2> GetBackBuffer(uint32_t Index);
	[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetBackBufferHandle(uint32_t Index) const;

private:
	HRESULT QueryBackBuffers();
	void DestroyBackBuffers();

private:
	FSwapChainDesc Desc;

	uint32_t Width = 0;
	uint32_t Height = 0;
	uint32_t SwapChainFlags = 0;
	ComPtr<IDXGISwapChain4> SwapChain;
	std::vector<ComPtr<ID3D12Resource2>> BackBuffers;
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> BackBufferHandles;
};
