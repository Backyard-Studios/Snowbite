#pragma once

#include <Engine/Core/Definitions.h>
#include <Engine/Renderer/D3D12Include.h>

#include "SwapChain.h"

struct SNOWBITE_API FFrameResource
{
	ComPtr<ID3D12CommandAllocator> CommandAllocator;
	ComPtr<ID3D12GraphicsCommandList9> CommandList;
	ComPtr<ID3D12Fence1> Fence;
	uint64_t FenceValue = 0;
	HANDLE FenceEvent = nullptr;
};

struct SNOWBITE_API FGraphicsDeviceDesc
{
	uint32_t MaxRenderTargets = 20;
};

class SNOWBITE_API FGraphicsDevice
{
public:
	FGraphicsDevice();
	~FGraphicsDevice();

	[[nodiscard]] HRESULT Initialize(const std::shared_ptr<FWindow>& InWindow, const FGraphicsDeviceDesc& InDesc);
	void Destroy();

	[[nodiscard]] HRESULT BeginFrame();
	[[nodiscard]] HRESULT EndFrame() const;

	[[nodiscard]] HRESULT Resize(uint32_t NewWidth, uint32_t NewHeight);

	HRESULT FlushFrames();

private:
#ifdef SB_DEBUG
	void EnableDebugLayer();
#endif
	HRESULT InitializeSDK(ComPtr<ID3D12DeviceFactory>& DeviceFactory) const;
	HRESULT ChooseAdapter(const ComPtr<ID3D12DeviceFactory>& DeviceFactory, D3D_FEATURE_LEVEL RequiredFeatureLevel,
	                      ComPtr<IDXGIAdapter4>& Adapter);

private:
	std::shared_ptr<FWindow> Window;
	FGraphicsDeviceDesc Desc;

#ifdef SB_DEBUG
	ComPtr<IDXGIDebug1> DebugController;
#endif
	ComPtr<IDXGIFactory7> Factory;
	ComPtr<IDXGIAdapter4> Adapter;
	DXGI_ADAPTER_DESC3 AdapterDesc = {};
	ComPtr<ID3D12Device13> Device;
	ComPtr<ID3D12CommandQueue> GraphicsCommandQueue;

	uint32_t CurrentFrameIndex = 0;
	constexpr static uint32_t BufferCount = 3;
	std::array<FFrameResource, BufferCount> FrameResources;

	ComPtr<ID3D12DescriptorHeap> RTVHeap;

	std::shared_ptr<FSwapChain> SwapChain;
};
