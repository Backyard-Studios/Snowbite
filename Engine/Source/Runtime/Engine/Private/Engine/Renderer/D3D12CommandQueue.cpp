#include "pch.h"

#include <Engine/Renderer/D3D12CommandQueue.h>

FD3D12CommandQueue::FD3D12CommandQueue(const std::shared_ptr<FD3D12Device>& InDevice)
	: ID3D12DeviceObject(InDevice), Type(D3D12_COMMAND_LIST_TYPE_NONE)
{
}

FD3D12CommandQueue::~FD3D12CommandQueue()
{
}

HRESULT FD3D12CommandQueue::Initialize(const D3D12_COMMAND_LIST_TYPE InType)
{
	Type = InType;
	D3D12_COMMAND_QUEUE_DESC CommandQueueDesc;
	CommandQueueDesc.NodeMask = 0;
	CommandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
	CommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	CommandQueueDesc.Type = Type;
	SB_CHECK(GetD3D12Device()->CreateCommandQueue(&CommandQueueDesc, IID_PPV_ARGS(&CommandQueue)));
	Fence = std::make_shared<FD3D12Fence>(GetDevice());
	SB_CHECK(Fence->Initialize());
	return S_OK;
}

void FD3D12CommandQueue::Destroy()
{
	SB_SAFE_DESTROY(Fence);
	CommandQueue.Reset();
}

void FD3D12CommandQueue::ExecuteCommandLists(const uint32_t Count, ID3D12CommandList* const* CommandLists) const
{
	CommandQueue->ExecuteCommandLists(Count, CommandLists);
}

HRESULT FD3D12CommandQueue::Signal() const
{
	SB_CHECK(CommandQueue->Signal(Fence->GetD3D12Fence().Get(), Fence->GetValue()));
	return S_OK;
}

HRESULT FD3D12CommandQueue::Signal(const std::shared_ptr<FD3D12Fence>& InFence) const
{
	if (InFence->IsCompleted())
		InFence->IncrementValue();
	SB_CHECK(InFence->SetEvent(InFence->GetValue()));
	SB_CHECK(CommandQueue->Signal(InFence->GetD3D12Fence().Get(), InFence->GetValue()));
	return S_OK;
}

HRESULT FD3D12CommandQueue::WaitForCompletion() const
{
	SB_CHECK(Fence->Wait());
	return S_OK;
}

HRESULT FD3D12CommandQueue::WaitForCompletion(const std::shared_ptr<FD3D12Fence>& InFence) const
{
	SB_CHECK(InFence->Wait());
	return S_OK;
}

HRESULT FD3D12CommandQueue::SignalAndWaitForCompletion() const
{
	SB_CHECK(Signal());
	SB_CHECK(WaitForCompletion());
	return S_OK;
}

HRESULT FD3D12CommandQueue::SignalAndWaitForCompletion(const std::shared_ptr<FD3D12Fence>& InFence) const
{
	SB_CHECK(Signal(InFence));
	SB_CHECK(WaitForCompletion(InFence));
	return S_OK;
}
