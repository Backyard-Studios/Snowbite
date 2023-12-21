#pragma once

#include <Engine/Core/Definitions.h>
#include <Engine/Graphics/D3D12/D3D12Include.h>

class SNOWBITE_API FD3D12DepthStencil
{
public:
	FD3D12DepthStencil(const ComPointer<ID3D12Device10>& InDevice, const ComPointer<D3D12MA::Allocator>& InAllocator,
	                   const ComPointer<ID3D12DescriptorHeap>& InDSVDescriptorHeap);
	~FD3D12DepthStencil();

	HRESULT Initialize(DXGI_FORMAT Format, uint32_t Width, uint32_t Height,
	                   const char* Name = "<unnamed D3D12DepthStencil>");
	void Destroy();

	[[nodiscard]] ComPointer<ID3D12Resource2> GetDepthStencil() const { return DepthStencil; }
	[[nodiscard]] ComPointer<D3D12MA::Allocation> GetAllocation() const { return Allocation; }
	[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle() const { return CpuHandle; }

private:
	ComPointer<ID3D12Device10> Device;
	ComPointer<D3D12MA::Allocator> Allocator;
	ComPointer<ID3D12DescriptorHeap> DSVDescriptorHeap;

	ComPointer<ID3D12Resource2> DepthStencil;
	ComPointer<D3D12MA::Allocation> Allocation;
	D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle;
};
