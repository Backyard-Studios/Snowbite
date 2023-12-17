#include "pch.h"

#include <Engine/Graphics/Fence.h>

#include "Engine/Utils/StringUtils.h"

FFence::FFence(ComPointer<ID3D12Device10> Device, const char* Name)
{
	const HRESULT Result = Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence));
	SB_D3D_ASSERT(Result, "Failed to create fence");
	Fence->SetName(StringConverter<wchar_t>()(Name).c_str());
	FenceValue = 0;
	FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

FFence::~FFence()
{
	Fence->Release();
	CloseHandle(FenceEvent);
}

void FFence::Signal(ComPointer<ID3D12CommandQueue> CommandQueue)
{
	++FenceValue;
	const HRESULT Result = CommandQueue->Signal(Fence.Get(), FenceValue);
	SB_D3D_ASSERT(Result, "Failed to signal fence");
}

void FFence::WaitOnCPU(const uint64_t FenceValue)
{
	if (Fence->GetCompletedValue() < FenceValue)
	{
		const HRESULT Result = Fence->SetEventOnCompletion(FenceValue, FenceEvent);
		SB_D3D_ASSERT(Result, "Failed to set event on completion");
		WaitForSingleObject(FenceEvent, INFINITE);
	}
}

void FFence::WaitOnGPU(ComPointer<ID3D12CommandQueue> CommandQueue)
{
	const HRESULT Result = CommandQueue->Wait(Fence.Get(), FenceValue);
	SB_D3D_ASSERT(Result, "Failed to wait on fence");
}
