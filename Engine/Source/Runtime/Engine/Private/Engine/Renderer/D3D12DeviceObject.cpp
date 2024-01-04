#include "pch.h"

#include <Engine/Renderer/D3D12DeviceObject.h>

ID3D12DeviceObject::ID3D12DeviceObject(const std::shared_ptr<FD3D12Device>& InDevice)
	: Device(InDevice)
{
}

ID3D12DeviceObject::~ID3D12DeviceObject()
{
	SB_SAFE_RESET(Device);
}
