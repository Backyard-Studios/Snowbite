#include "pch.h"

#include <Engine/Graphics/D3D12/D3D12CommandList.h>

#include "Engine/Graphics/D3D12/D3D12CommandAllocator.h"
#include "Engine/Graphics/D3D12/D3D12CommandListType.h"
#include "Engine/Graphics/D3D12/D3D12GraphicsDevice.h"

FD3D12CommandList::FD3D12CommandList()
{
}

FD3D12CommandList::~FD3D12CommandList()
{
}

HRESULT FD3D12CommandList::Initialize(const std::shared_ptr<IGraphicsDevice> InDevice, const ECommandListType InType)
{
	const ComPtr<ID3D12Device13> Device = InDevice->As<FD3D12GraphicsDevice>()->GetNativeDevice();
	const HRESULT CreateResult = Device->CreateCommandList1(0, ConvertCommandListType(InType),
	                                                        D3D12_COMMAND_LIST_FLAG_NONE,
	                                                        IID_PPV_ARGS(&CommandList));
	SB_CHECK_RESULT_LOW(CreateResult);
	return S_OK;
}

void FD3D12CommandList::Destroy()
{
	CommandList.Reset();
}

HRESULT FD3D12CommandList::Reset(const std::shared_ptr<ICommandAllocator> InCommandAllocator)
{
	const ComPtr<ID3D12CommandAllocator> CommandAllocator = InCommandAllocator->As<FD3D12CommandAllocator>()->
		GetNativeCommandAllocator();
	const HRESULT ResetResult = CommandList->Reset(CommandAllocator.Get(), nullptr);
	SB_CHECK_RESULT_LOW(ResetResult);
	return S_OK;
}

HRESULT FD3D12CommandList::Close()
{
	const HRESULT CloseResult = CommandList->Close();
	SB_CHECK_RESULT_LOW(CloseResult);
	return S_OK;
}
