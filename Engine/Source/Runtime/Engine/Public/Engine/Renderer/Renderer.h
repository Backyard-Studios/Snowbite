#pragma once

#include <Engine/Core/Definitions.h>
#include <Engine/Renderer/D3D12Include.h>

class SNOWBITE_API FRenderer
{
public:

private:
	[[nodiscard]] static HRESULT Initialize(const std::shared_ptr<FWindow>& InWindow);
	static void Shutdown();

	static void BeginFrame();
	static void EndFrame();

private:
	static HRESULT InitializeSDK(ComPtr<ID3D12SDKConfiguration1>& SDKConfig,
	                             ComPtr<ID3D12DeviceFactory>& DeviceFactory);
#ifdef SB_DEBUG
	static void InitializeDebugLayer();
#endif
	static HRESULT InitializeFactory();
	static HRESULT ChooseAdapter(const ComPtr<ID3D12DeviceFactory>& DeviceFactory, ComPtr<IDXGIAdapter4>& TempAdapter);

private:
	static std::shared_ptr<FWindow> Window;

#ifdef SB_DEBUG
	static ComPtr<IDXGIDebug1> DXGIDebug;
#endif
	static ComPtr<IDXGIFactory7> Factory;
	static ComPtr<ID3D12Device13> Device;
	static ComPtr<ID3D12CommandQueue> CommandQueue;

	friend class FEngine;
};
