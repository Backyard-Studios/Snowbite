#pragma once

#include <Engine/Core/Definitions.h>
#include <Engine/Graphics/DirectXInclude.h>

class SNOWBITE_API FFence
{
public:
	FFence(ComPointer<ID3D12Device10> Device, const char* Name = "<unnamed FFence>");
	~FFence();

	void Signal(ComPointer<ID3D12CommandQueue> CommandQueue);
	void WaitOnCPU(uint64_t FenceValue);
	void WaitOnGPU(ComPointer<ID3D12CommandQueue> CommandQueue);

	[[nodiscard]] uint64_t GetFenceValue() const { return FenceValue; }

private:
	ComPointer<ID3D12Fence> Fence;
	uint64_t FenceValue;
	HANDLE FenceEvent;
};
