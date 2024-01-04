#pragma once

#include <Engine/Core/Definitions.h>

class SNOWBITE_API FD3D12Device
{
public:
	FD3D12Device(const ComPtr<IDXGIFactory7>& InFactory);
	~FD3D12Device();

	[[nodiscard]] HRESULT Initialize(const ComPtr<IDXGIAdapter4>& Adapter,
	                                 const ComPtr<ID3D12DeviceFactory>& DeviceFactory);
	void Destroy();

	[[nodiscard]] ComPtr<IDXGIFactory7> GetFactory() const { return Factory; };
	[[nodiscard]] ComPtr<ID3D12Device13> GetD3D12Device() const { return Device; }

private:
	ComPtr<IDXGIFactory7> Factory;
	ComPtr<ID3D12Device13> Device;
};
