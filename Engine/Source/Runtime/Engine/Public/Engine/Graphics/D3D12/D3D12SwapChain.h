#pragma once
#include "Engine/Graphics/SwapChain.h"

class FD3D12SwapChain : public ISwapChain
{
public:
	FD3D12SwapChain();
	~FD3D12SwapChain() override;

	[[nodiscard]] HRESULT
	Initialize(std::shared_ptr<IGraphicsInterface> InGraphicsInterface, HWND WindowHandle) override;
	void Destroy() override;

	[[nodiscard]] HRESULT Present(bool bShouldVSync) override;
	void Resize(const FUInt2& InSize) override;
	[[nodiscard]] uint32_t GetBackBufferIndex() override;
	[[nodiscard]] FUInt2 GetBackBufferSize() override { return BackBufferSize; }

	[[nodiscard]] void* GetNativePointer() override { return SwapChain.Get(); }

	[[nodiscard]] ComPtr<IDXGISwapChain4> GetNativeSwapChain() const { return SwapChain; }
	[[nodiscard]] ComPtr<ID3D12Resource2> GetBackBuffer(const uint32_t Index) const { return BackBuffers.at(Index); }

	[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetBackBufferHandle(const uint32_t Index) const
	{
		return BackBufferHandles.at(Index);
	}

	[[nodiscard]] ComPtr<ID3D12Resource2> GetCurrentBackBuffer() { return BackBuffers.at(GetBackBufferIndex()); }

	[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferHandle()
	{
		return BackBufferHandles.at(GetBackBufferIndex());
	}

private:
	HRESULT QueryBackBuffers();
	void DestroyBackBuffers();

private:
	ComPtr<ID3D12Device13> Device;

	uint32_t SwapChainFlags = 0;
	ComPtr<IDXGISwapChain4> SwapChain;

	ComPtr<ID3D12DescriptorHeap> BackBufferDescriptorHeap;

	uint32_t BackBufferCount = 0;
	FUInt2 BackBufferSize = {0, 0};
	std::vector<ComPtr<ID3D12Resource2>> BackBuffers;
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> BackBufferHandles;
};
