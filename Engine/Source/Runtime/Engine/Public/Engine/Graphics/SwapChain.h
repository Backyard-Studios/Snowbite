#pragma once

#include <Engine/Core/Definitions.h>
#include <Engine/Graphics/DirectXInclude.h>

#include "Window.h"

class FGraphicsDevice;

struct SNOWBITE_API FSwapChainDesc
{
	uint32_t Width = 0;
	uint32_t Height = 0;
	uint32_t BufferCount = 0;
	DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	std::shared_ptr<FWindow> Window;
};

class SNOWBITE_API FSwapChain
{
public:
	FSwapChain(FGraphicsDevice* InGraphicsDevice, const FSwapChainDesc& InDesc);
	~FSwapChain();

	void Resize(uint32_t InWidth, uint32_t InHeight);
	void Present(bool bShouldVSync = true);

	[[nodiscard]] uint32_t GetFrameIndex();
	[[nodiscard]] ComPointer<ID3D12Resource2> GetBackBuffer();
	[[nodiscard]] ComPointer<ID3D12Resource2> GetBackBuffer(uint32_t Index) const;
	[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetBackBufferDescriptor();
	[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetBackBufferDescriptor(uint32_t Index) const;

private:
	void QueryBackBuffers();
	void ReleaseBackBuffers();

private:
	FGraphicsDevice* GraphicsDevice;
	FSwapChainDesc Desc;

	uint32_t SwapChainCreationFlags = 0;
	ComPointer<IDXGISwapChain4> SwapChain;
	std::vector<ComPointer<ID3D12Resource2>> BackBuffers;
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> BackBufferDescriptors;
};
