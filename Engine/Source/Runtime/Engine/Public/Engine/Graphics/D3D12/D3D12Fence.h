#pragma once

#include <Engine/Core/Definitions.h>
#include <Engine/Graphics/D3D12/D3D12Include.h>

class SNOWBITE_API FD3D12Fence
{
public:
	FD3D12Fence(const ComPointer<ID3D12Device10>& InDevice, const char* InName = "<unnamed D3D12Fence>");
	~FD3D12Fence();

	HRESULT Initialize();
	void Destroy();

	HRESULT Signal(ComPointer<ID3D12CommandQueue> CommandQueue);
	HRESULT WaitOnCPU();
	HRESULT WaitOnCPU(uint64_t ExternalFenceValue);
	HRESULT WaitOnGPU(ComPointer<ID3D12CommandQueue> CommandQueue);

	[[nodiscard]] uint64_t GetFenceValue() const { return FenceValue; }

private:
	ComPointer<ID3D12Device10> Device;
	const char* Name;
	ComPointer<ID3D12Fence1> Fence;
	uint64_t FenceValue = 0;
	HANDLE FenceEvent;
};
