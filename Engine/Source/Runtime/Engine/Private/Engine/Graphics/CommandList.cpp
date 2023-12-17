#include "pch.h"

#include <Engine/Graphics/CommandList.h>

#include "Engine/Utils/StringUtils.h"

FCommandList::FCommandList(ComPointer<ID3D12Device10> Device, const D3D12_COMMAND_LIST_TYPE Type,
                           const bool bShouldAutoClose, const char* Name)
{
	const HRESULT AllocatorCreateResult = Device->CreateCommandAllocator(Type, IID_PPV_ARGS(&CommandAllocator));
	SB_D3D_ASSERT(AllocatorCreateResult, "Failed to create command allocator");
	const HRESULT ListCreateResult = Device->CreateCommandList(0, Type, CommandAllocator.Get(), nullptr,
	                                                           IID_PPV_ARGS(&CommandList));
	SB_D3D_ASSERT(ListCreateResult, "Failed to create command list");
	if (bShouldAutoClose)
		CommandList->Close();
	CommandList->SetName(StringConverter<wchar_t>()(Name).c_str());
}

FCommandList::~FCommandList()
{
	CommandList.Release();
	CommandAllocator.Release();
}

void FCommandList::Reset(ComPointer<ID3D12PipelineState> InitialState)
{
	const HRESULT AllocatorResetResult = CommandAllocator->Reset();
	SB_D3D_ASSERT(AllocatorResetResult, "Failed to reset command allocator");
	const HRESULT ListResetResult = CommandList->Reset(CommandAllocator.Get(), InitialState);
	SB_D3D_ASSERT(ListResetResult, "Failed to reset command list");
}

void FCommandList::Close()
{
	const HRESULT ListCloseResult = CommandList->Close();
	SB_D3D_ASSERT(ListCloseResult, "Failed to close command list");
}

void FCommandList::Execute(ComPointer<ID3D12CommandQueue> CommandQueue, const std::shared_ptr<FFence>& Fence)
{
	Close();
	ID3D12CommandList* CommandLists[] = {CommandList.Get()};
	CommandQueue->ExecuteCommandLists(1, CommandLists);
	Fence->Signal(CommandQueue);
	Fence->WaitOnCPU(Fence->GetFenceValue());
}
