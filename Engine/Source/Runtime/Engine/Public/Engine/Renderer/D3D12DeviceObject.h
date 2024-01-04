#pragma once

#include <Engine/Core/Definitions.h>

#include "D3D12Device.h"

/**
 * @brief Base class for all D3D12 objects that are created through the device
 * @note Similar to ID3D12DeviceChild.
 */
class SNOWBITE_API ID3D12DeviceObject
{
public:
	ID3D12DeviceObject(const std::shared_ptr<FD3D12Device>& InDevice);
	virtual ~ID3D12DeviceObject();

	[[nodiscard]] std::shared_ptr<FD3D12Device> GetDevice() const { return Device; }
	[[nodiscard]] ComPtr<IDXGIFactory7> GetFactory() const { return Device->GetFactory(); }
	[[nodiscard]] ComPtr<ID3D12Device13> GetD3D12Device() const { return Device->GetD3D12Device(); }

private:
	std::shared_ptr<FD3D12Device> Device;
};
