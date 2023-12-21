#include "pch.h"

#include <Engine/Graphics/D3D12/D3D12CommandList.h>

#include "Engine/Graphics/D3D12/D3D12Utils.h"
#include "Engine/Utils/StringUtils.h"

FD3D12CommandList::FD3D12CommandList(const ComPointer<ID3D12Device10>& InDevice, const D3D12_COMMAND_LIST_TYPE InType,
                                     const char* InName)
	: Device(InDevice), Type(InType), Name(InName)
{
}

FD3D12CommandList::~FD3D12CommandList() = default;

HRESULT FD3D12CommandList::Initialize()
{
	const HRESULT CreateAllocatorResult = Device->CreateCommandAllocator(Type, IID_PPV_ARGS(&CommandAllocator));
	SB_D3D_ASSERT_RETURN(CreateAllocatorResult, "Unable to create command allocator");
	const HRESULT CreateListResult = Device->CreateCommandList(0, Type, CommandAllocator, nullptr,
	                                                           IID_PPV_ARGS(&CommandList));
	SB_D3D_ASSERT_RETURN(CreateListResult, "Unable to create command list");
	CommandList->SetName(StringToWideChar(Name));
	return S_OK;
}

void FD3D12CommandList::Destroy()
{
	CommandList.Release();
	CommandAllocator.Release();
}

HRESULT FD3D12CommandList::Reset(ComPointer<ID3D12PipelineState> InitialState)
{
	const HRESULT AllocatorResetResult = CommandAllocator->Reset();
	SB_D3D_ASSERT_RETURN(AllocatorResetResult, "Unable to reset command allocator");
	const HRESULT ListResetResult = CommandList->Reset(CommandAllocator, InitialState);
	SB_D3D_ASSERT_RETURN(ListResetResult, "Unable to reset command list");
	return S_OK;
}

HRESULT FD3D12CommandList::Close()
{
	const HRESULT CloseResult = CommandList->Close();
	SB_D3D_ASSERT_RETURN(CloseResult, "Unable to close command list");
	return S_OK;
}

HRESULT FD3D12CommandList::Execute(ComPointer<ID3D12CommandQueue> CommandQueue,
                                   const std::shared_ptr<FD3D12Fence>& Fence)
{
	Close();
	ID3D12CommandList* CommandLists[] = {CommandList.Get()};
	CommandQueue->ExecuteCommandLists(1, CommandLists);
	const HRESULT SignalResult = Fence->Signal(CommandQueue);
	SB_D3D_FAILED_RETURN(SignalResult);
	const HRESULT WaitResult = Fence->WaitOnCPU();
	SB_D3D_FAILED_RETURN(WaitResult);
	return S_OK;
}
