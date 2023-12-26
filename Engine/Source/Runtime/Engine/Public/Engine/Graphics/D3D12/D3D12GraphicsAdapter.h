#pragma once

#include <Engine/Graphics/GraphicsAdapter.h>
#include <Engine/Graphics/D3D12/D3D12Include.h>

class FD3D12GraphicsAdapter : public IGraphicsAdapter
{
public:
	FD3D12GraphicsAdapter(const ComPtr<IDXGIAdapter4>& InAdapter);
	~FD3D12GraphicsAdapter() override;

	void Destroy() override;

	[[nodiscard]] void* GetNativePointer() const override { return Adapter.Get(); }

	[[nodiscard]] ComPtr<IDXGIAdapter4> GetNativeAdapter() const { return Adapter; }
	[[nodiscard]] DXGI_ADAPTER_DESC3 GetDesc() const { return Desc; }
	[[nodiscard]] D3D_FEATURE_LEVEL GetMaxSupportedFeatureLevel() const { return MaxSupportedFeatureLevel; }

private:
	ComPtr<IDXGIAdapter4> Adapter;
	DXGI_ADAPTER_DESC3 Desc;
	D3D_FEATURE_LEVEL MaxSupportedFeatureLevel = D3D_FEATURE_LEVEL_11_0;
};
