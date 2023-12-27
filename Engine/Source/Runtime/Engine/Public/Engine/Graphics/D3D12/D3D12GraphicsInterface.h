#pragma once

#include <Engine/Graphics/GraphicsInterface.h>
#include <Engine/Graphics/D3D12/D3D12Include.h>

#include "Engine/Graphics/GraphicsDevice.h"

class FD3D12GraphicsInterface : public IGraphicsInterface
{
public:
	FD3D12GraphicsInterface();
	~FD3D12GraphicsInterface() override;

	[[nodiscard]] HRESULT BeginFrame() override;
	[[nodiscard]] HRESULT EndFrame() override;

	[[nodiscard]] ComPtr<IDXGIFactory7> GetFactory() const { return Factory; }
	[[nodiscard]] std::shared_ptr<IGraphicsDevice> GetDevice() const { return Device; }

private:
	[[nodiscard]] HRESULT Initialize(std::shared_ptr<FWindow> InWindow) override;
	void Destroy() override;

	[[nodiscard]] HRESULT InitializeDeviceContext(const std::shared_ptr<IGraphicsDevice>& InDevice) const;

private:
	std::shared_ptr<FWindow> Window;

#if SB_D3D12_ENABLE_DEBUG_LAYER
	ComPtr<IDXGIDebug1> DXGIDebug;
#endif
	ComPtr<IDXGIFactory7> Factory;

	std::shared_ptr<IGraphicsDevice> Device;

	uint32_t BackBufferIndex = 0;
};
