#pragma once

#include "D3D12GraphicsAdapter.h"
#include "Engine/Graphics/GraphicsDevice.h"

class FD3D12GraphicsDevice : public IGraphicsDevice
{
public:
	FD3D12GraphicsDevice(const std::shared_ptr<FD3D12GraphicsAdapter>& InAdapter);
	~FD3D12GraphicsDevice() override;

	[[nodiscard]] HRESULT Initialize() override;
	void Destroy() override;

	[[nodiscard]] void* GetNativePointer() const override { return Device.Get(); }
	[[nodiscard]] std::shared_ptr<IGraphicsAdapter> GetAdapter() const override { return Adapter; }

	[[nodiscard]] ComPtr<ID3D12Device13> GetNativeDevice() const { return Device; }

private:
	std::shared_ptr<FD3D12GraphicsAdapter> Adapter;

	ComPtr<ID3D12Device13> Device;
};
