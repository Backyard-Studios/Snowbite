#include "pch.h"

#include <Engine/Graphics/D3D12/D3D12DepthStencil.h>

#include "Engine/Utils/StringUtils.h"

FD3D12DepthStencil::FD3D12DepthStencil(const ComPointer<ID3D12Device10>& InDevice,
                                       const ComPointer<D3D12MA::Allocator>& InAllocator,
                                       const ComPointer<ID3D12DescriptorHeap>& InDSVDescriptorHeap)
	: Device(InDevice), Allocator(InAllocator), DSVDescriptorHeap(InDSVDescriptorHeap), DepthStencil(nullptr),
	  Allocation(nullptr), CpuHandle{}
{
}

FD3D12DepthStencil::~FD3D12DepthStencil() = default;

HRESULT FD3D12DepthStencil::Initialize(const DXGI_FORMAT Format, const uint32_t Width, const uint32_t Height,
                                       const char* Name)
{
	D3D12MA::ALLOCATION_DESC AllocationDesc = {};
	AllocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_RESOURCE_DESC ResourceDesc;
	ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	ResourceDesc.Alignment = 0;
	ResourceDesc.Width = Width;
	ResourceDesc.Height = Height;
	ResourceDesc.DepthOrArraySize = 1;
	ResourceDesc.MipLevels = 1;
	ResourceDesc.Format = Format;
	ResourceDesc.SampleDesc.Count = 1;
	ResourceDesc.SampleDesc.Quality = 0;
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE ClearValue;
	ClearValue.Format = Format;
	ClearValue.DepthStencil.Depth = 1.0f;
	ClearValue.DepthStencil.Stencil = 0;
	const HRESULT CreateResult = Allocator->CreateResource(&AllocationDesc, &ResourceDesc,
	                                                       D3D12_RESOURCE_STATE_DEPTH_WRITE, &ClearValue, &Allocation,
	                                                       IID_PPV_ARGS(&DepthStencil));
	SB_D3D_ASSERT(CreateResult, "Failed to create depth stencil");
	DepthStencil->SetName(StringToWideChar(Name));

	D3D12_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc;
	DepthStencilViewDesc.Format = Format;
	DepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	DepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;
	DepthStencilViewDesc.Texture2D.MipSlice = 0;

	const D3D12_CPU_DESCRIPTOR_HANDLE DSVDescriptor = DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	Device->CreateDepthStencilView(DepthStencil.Get(), &DepthStencilViewDesc, DSVDescriptor);
	return S_OK;
}

void FD3D12DepthStencil::Destroy()
{
	Allocation.Release();
	DepthStencil.Release();
	CpuHandle = {};
}
