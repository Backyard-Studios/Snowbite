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

private:
	[[nodiscard]] HRESULT Initialize() override;
	void Destroy() override;

	[[nodiscard]] HRESULT InitializeDeviceContext(const std::shared_ptr<IGraphicsDevice>& InDevice) const;

private:
#if SB_D3D12_ENABLE_DEBUG_LAYER
	ComPtr<IDXGIDebug1> DXGIDebug;
#endif
	ComPtr<IDXGIFactory7> Factory;

	std::shared_ptr<IGraphicsDevice> Device;
};
