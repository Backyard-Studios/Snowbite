#pragma once

#include <Engine/Graphics/GraphicsInterface.h>
#include <Engine/Graphics/D3D12/D3D12Include.h>

class FD3D12GraphicsInterface : public IGraphicsInterface
{
public:
	FD3D12GraphicsInterface();
	~FD3D12GraphicsInterface() override;

private:
	[[nodiscard]] HRESULT Initialize() override;
	void Destroy() override;

private:
#if SB_D3D12_ENABLE_DEBUG_LAYER
	ComPtr<IDXGIDebug1> DXGIDebug;
#endif
	ComPtr<IDXGIFactory7> Factory;
};
