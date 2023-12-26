#include "pch.h"

#include <Engine/Graphics/D3D12/D3D12Fence.h>

#include "Engine/Graphics/D3D12/D3D12CommandQueue.h"
#include "Engine/Graphics/D3D12/D3D12GraphicsDevice.h"

FD3D12Fence::FD3D12Fence()
{
}

HRESULT FD3D12Fence::Initialize(const std::shared_ptr<IGraphicsDevice> InDevice)
{
	const ComPtr<ID3D12Device13> Device = InDevice->As<FD3D12GraphicsDevice>()->GetNativeDevice();
	const HRESULT CreateResult = Device->CreateFence(Value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(Fence.GetAddressOf()));
	SB_CHECK_RESULT_LOW(CreateResult);
	FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	return S_OK;
}

void FD3D12Fence::Destroy()
{
	if (FenceEvent)
	{
		CloseHandle(FenceEvent);
		FenceEvent = nullptr;
	}
	Fence.Reset();
}

HRESULT FD3D12Fence::Signal(const std::shared_ptr<ICommandQueue> InQueue)
{
	const ComPtr<ID3D12CommandQueue> Queue = InQueue->As<FD3D12CommandQueue>()->GetNativeCommandQueue();
	Value++;
	const HRESULT SignalResult = Queue->Signal(Fence.Get(), Value);
	SB_CHECK_RESULT_LOW(SignalResult);
	return S_OK;
}

HRESULT FD3D12Fence::Wait()
{
	if (Fence->GetCompletedValue() < Value)
	{
		const HRESULT SetEventResult = Fence->SetEventOnCompletion(Value, FenceEvent);
		SB_CHECK_RESULT_LOW(SetEventResult);
		const DWORD WaitResult = WaitForSingleObject(FenceEvent, 20000);
		if (WaitResult != WAIT_OBJECT_0)
			FPlatform::Fatal(WaitResult);
	}
	return S_OK;
}
