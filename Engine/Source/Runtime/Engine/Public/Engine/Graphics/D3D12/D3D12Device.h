#pragma once

#include <Engine/Core/Definitions.h>

#include "D3D12CommandList.h"
#include "D3D12Fence.h"
#include "D3D12MemAlloc.h"

struct SNOWBITE_API FD3D12AdapterRequirements
{
	bool bShouldBeHardwareAdapter = true;
	D3D_FEATURE_LEVEL MinimumFeatureLevel = D3D_FEATURE_LEVEL_12_0;
};

struct SNOWBITE_API FD3D12DeviceDesc
{
	const char* Name = "<unnamed D3D12Device>";
	FD3D12AdapterRequirements AdapterRequirements;
};

class SNOWBITE_API FD3D12Device
{
public:
	FD3D12Device(const ComPointer<IDXGIFactory7>& InFactory, const FD3D12DeviceDesc& InDesc);
	~FD3D12Device();

	HRESULT Initialize();
	void Destroy();

	D3D_SHADER_MODEL GetHighestSupportedShaderModel(D3D_SHADER_MODEL PreferredShaderModel = D3D_SHADER_MODEL_6_7);
	bool IsFormatSupported(DXGI_FORMAT Format, D3D12_FORMAT_SUPPORT1 Support1,
	                       D3D12_FORMAT_SUPPORT2 Support2 = D3D12_FORMAT_SUPPORT2_NONE);

	HRESULT CreateCommandQueue(ComPointer<ID3D12CommandQueue>& CommandQueue, D3D12_COMMAND_LIST_TYPE Type,
	                           const char* Name = "<unnamed D3D12CommandQueue>");
	std::shared_ptr<FD3D12CommandList> CreateCommandList(D3D12_COMMAND_LIST_TYPE Type,
	                                                     const char* Name = "<unnamed D3D12CommandList>");
	std::shared_ptr<FD3D12Fence> CreateFence(const char* Name = "<unnamed D3D12Fence>");

	[[nodiscard]] ComPointer<IDXGIAdapter4> GetAdapter() const { return Adapter; }
	[[nodiscard]] ComPointer<ID3D12Device10> GetNativeDevice() const { return Device; }
	[[nodiscard]] ComPointer<D3D12MA::Allocator> GetAllocator() const { return Allocator; }

	[[nodiscard]] D3D_SHADER_MODEL GetShaderModel() const { return ShaderModel; }
	[[nodiscard]] DXGI_FORMAT GetRenderTargetViewFormat() const { return RenderTargetViewFormat; }
	[[nodiscard]] DXGI_FORMAT GetDepthStencilViewFormat() const { return DepthStencilViewFormat; }

private:
	ComPointer<IDXGIAdapter4> GetSuitableAdapter(const FD3D12AdapterRequirements& Requirements);

private:
	ComPointer<IDXGIFactory7> Factory;
	FD3D12DeviceDesc Desc;

	ComPointer<IDXGIAdapter4> Adapter;
	DXGI_ADAPTER_DESC1 AdapterDesc;
	ComPointer<ID3D12Device10> Device;
	ComPointer<D3D12MA::Allocator> Allocator;

	D3D_SHADER_MODEL ShaderModel = D3D_SHADER_MODEL_5_1;

	DXGI_FORMAT RenderTargetViewFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT DepthStencilViewFormat = DXGI_FORMAT_D32_FLOAT;
};
