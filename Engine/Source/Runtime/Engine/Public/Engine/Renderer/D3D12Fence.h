#pragma once

#include <Engine/Core/Definitions.h>

#include "D3D12DeviceObject.h"

class SNOWBITE_API FD3D12Fence : public ID3D12DeviceObject
{
public:
	FD3D12Fence(const std::shared_ptr<FD3D12Device>& InDevice);
	~FD3D12Fence() override;

	[[nodiscard]] HRESULT Initialize();
	void Destroy();

	[[nodiscard]] HRESULT Signal();
	[[nodiscard]] HRESULT Wait(bool AdvanceValue = true);
	[[nodiscard]] HRESULT WaitForValue();
	void IncrementValue();
	[[nodiscard]] bool IsCompleted() const;
	[[nodiscard]] bool IsCompleted(uint64_t Value) const;

	[[nodiscard]] ComPtr<ID3D12Fence1> GetD3D12Fence() const { return Fence; }
	[[nodiscard]] uint64_t GetValue() const { return Value; }
	[[nodiscard]] HANDLE GetFenceEvent() const { return FenceEvent; }

	[[nodiscard]] uint64_t GetCompletedValue() const;

private:
	[[nodiscard]] HRESULT SetEvent(uint64_t Value) const;

private:
	ComPtr<ID3D12Fence1> Fence;
	uint64_t Value;
	HANDLE FenceEvent;

	friend class FD3D12CommandQueue;
};
