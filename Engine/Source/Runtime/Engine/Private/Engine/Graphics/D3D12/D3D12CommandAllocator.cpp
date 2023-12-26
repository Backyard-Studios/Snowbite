#include "pch.h"

#include <Engine/Graphics/D3D12/D3D12CommandAllocator.h>

#include "Engine/Graphics/D3D12/D3D12CommandListType.h"
#include "Engine/Graphics/D3D12/D3D12GraphicsDevice.h"

FD3D12CommandAllocator::FD3D12CommandAllocator()
{
}

FD3D12CommandAllocator::~FD3D12CommandAllocator()
{
}

HRESULT FD3D12CommandAllocator::Initialize(const std::shared_ptr<IGraphicsDevice> InDevice,
                                           const ECommandListType InType)
{
	const ComPtr<ID3D12Device13> Device = InDevice->As<FD3D12GraphicsDevice>()->GetNativeDevice();
	const HRESULT CreateResult = Device->CreateCommandAllocator(ConvertCommandListType(InType),
	                                                            IID_PPV_ARGS(&CommandAllocator));
	SB_CHECK_RESULT_LOW(CreateResult);
	return S_OK;
}

void FD3D12CommandAllocator::Destroy()
{
	CommandAllocator.Reset();
}

HRESULT FD3D12CommandAllocator::Reset()
{
	const HRESULT ResetResult = CommandAllocator->Reset();
	SB_CHECK_RESULT_LOW(ResetResult);
	return S_OK;
}
