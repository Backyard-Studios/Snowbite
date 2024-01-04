#include "pch.h"

#include <Engine/Renderer/D3D12Fence.h>

FD3D12Fence::FD3D12Fence(const std::shared_ptr<FD3D12Device>& InDevice)
	: ID3D12DeviceObject(InDevice), Value(0), FenceEvent(nullptr)
{
}

FD3D12Fence::~FD3D12Fence()
{
}

HRESULT FD3D12Fence::Initialize()
{
	SB_CHECK(GetD3D12Device()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));
	return S_OK;
}

void FD3D12Fence::Destroy()
{
	if (FenceEvent)
		CloseHandle(FenceEvent);
	Fence.Reset();
}

HRESULT FD3D12Fence::Signal()
{
	if (IsCompleted())
		IncrementValue();
	SB_CHECK(SetEvent(Value));
	SB_CHECK(Fence->Signal(Value));
	return S_OK;
}

HRESULT FD3D12Fence::Wait(const bool AdvanceValue)
{
	SB_CHECK(WaitForValue());
	if (AdvanceValue)
		IncrementValue();
	return S_OK;
}

HRESULT FD3D12Fence::WaitForValue()
{
	const DWORD WaitResult = WaitForSingleObject(FenceEvent, 20000);
	if (WaitResult != WAIT_OBJECT_0)
		return DXGI_ERROR_WAIT_TIMEOUT;
	IncrementValue();
	return S_OK;
}

void FD3D12Fence::IncrementValue()
{
	Value++;
}

bool FD3D12Fence::IsCompleted() const
{
	return IsCompleted(Value);
}

bool FD3D12Fence::IsCompleted(const uint64_t Value) const
{
	return Fence->GetCompletedValue() >= Value;
}

uint64_t FD3D12Fence::GetCompletedValue() const
{
	return Fence->GetCompletedValue();
}

HRESULT FD3D12Fence::SetEvent(const uint64_t Value) const
{
	return Fence->SetEventOnCompletion(Value, FenceEvent);
}
