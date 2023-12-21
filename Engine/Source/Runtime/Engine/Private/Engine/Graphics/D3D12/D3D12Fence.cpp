#include "pch.h"

#include <Engine/Graphics/D3D12/D3D12Fence.h>

#include "Engine/Graphics/D3D12/D3D12Utils.h"
#include "Engine/Utils/StringUtils.h"

FD3D12Fence::FD3D12Fence(const ComPointer<ID3D12Device10>& InDevice, const char* InName)
	: Device(InDevice), Name(InName), FenceEvent(nullptr)
{
}

FD3D12Fence::~FD3D12Fence() = default;

HRESULT FD3D12Fence::Initialize()
{
	const HRESULT FenceCreateResult = Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence));
	SB_D3D_ASSERT_RETURN(FenceCreateResult, "Failed to create fence");
	Fence->SetName(StringToWideChar(Name));
	FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	FenceValue = 0;
	return S_OK;
}

void FD3D12Fence::Destroy()
{
	Device.Release();
	Fence.Release();
	if (FenceEvent)
		CloseHandle(FenceEvent);
}

HRESULT FD3D12Fence::Signal(ComPointer<ID3D12CommandQueue> CommandQueue)
{
	++FenceValue;
	const HRESULT Result = CommandQueue->Signal(Fence.Get(), FenceValue);
	SB_D3D_ASSERT_RETURN(Result, "Failed to signal fence");
	return S_OK;
}

HRESULT FD3D12Fence::WaitOnCPU()
{
	if (Fence->GetCompletedValue() < FenceValue)
	{
		const HRESULT Result = Fence->SetEventOnCompletion(FenceValue, FenceEvent);
		SB_D3D_ASSERT_RETURN(Result, "Failed to set event on completion");
		WaitForSingleObject(FenceEvent, INFINITE);
	}
	return S_OK;
}

HRESULT FD3D12Fence::WaitOnCPU(const uint64_t ExternalFenceValue)
{
	if (Fence->GetCompletedValue() < ExternalFenceValue)
	{
		const HRESULT Result = Fence->SetEventOnCompletion(ExternalFenceValue, FenceEvent);
		SB_D3D_ASSERT_RETURN(Result, "Failed to set event on completion");
		WaitForSingleObject(FenceEvent, INFINITE);
	}
	return S_OK;
}

HRESULT FD3D12Fence::WaitOnGPU(ComPointer<ID3D12CommandQueue> CommandQueue)
{
	const HRESULT Result = CommandQueue->Wait(Fence.Get(), FenceValue);
	SB_D3D_ASSERT_RETURN(Result, "Failed to wait on fence");
	return S_OK;
}
