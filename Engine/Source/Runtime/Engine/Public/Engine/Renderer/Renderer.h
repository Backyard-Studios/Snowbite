#pragma once

#include <Engine/Core/Definitions.h>
#include <Engine/Renderer/D3D12Include.h>

#include "SwapChain.h"

struct SNOWBITE_API FFrameContext
{
	ComPtr<ID3D12CommandAllocator> CommandAllocator;
	ComPtr<ID3D12GraphicsCommandList9> CommandList;
	ComPtr<ID3D12Fence1> Fence;
	uint64_t FenceValue;
	HANDLE FenceEvent;
};

class SNOWBITE_API FRenderer
{
public:

private:
	[[nodiscard]] static HRESULT Initialize(const std::shared_ptr<FWindow>& InWindow);
	static void Shutdown();

	[[nodiscard]] static HRESULT Resize(uint32_t Width, uint32_t Height);

	[[nodiscard]] static HRESULT BeginFrame();
	[[nodiscard]] static HRESULT EndFrame();

	[[nodiscard]] static HRESULT WaitForFence(const ComPtr<ID3D12Fence1>& Fence, uint64_t FenceValue,
	                                          HANDLE FenceEvent);
	[[nodiscard]] static HRESULT WaitForFrame(uint32_t Index);
	[[nodiscard]] static HRESULT FlushFrames();

private:
	[[nodiscard]] static HRESULT InitializeSDK(ComPtr<ID3D12SDKConfiguration1>& SDKConfig,
	                                           ComPtr<ID3D12DeviceFactory>& DeviceFactory);
#ifdef SB_DEBUG
	static void InitializeDebugLayer();
#endif
	[[nodiscard]] static HRESULT InitializeFactory();
	[[nodiscard]] static HRESULT ChooseAdapter(const ComPtr<ID3D12DeviceFactory>& DeviceFactory,
	                                           ComPtr<IDXGIAdapter4>& TempAdapter);

private:
	static std::shared_ptr<FWindow> Window;

	static constexpr uint32_t BufferCount = 3;
	static uint32_t CurrentFrameIndex;
	static bool bIsResizing;

#ifdef SB_DEBUG
	static ComPtr<IDXGIDebug1> DXGIDebug;
#endif
	static ComPtr<IDXGIFactory7> Factory;
	static ComPtr<ID3D12Device13> Device;
	static ComPtr<ID3D12CommandQueue> CommandQueue;

	static std::shared_ptr<FSwapChain> SwapChain;
	static std::array<FFrameContext, BufferCount> FrameContexts;

	friend class FEngine;
};
