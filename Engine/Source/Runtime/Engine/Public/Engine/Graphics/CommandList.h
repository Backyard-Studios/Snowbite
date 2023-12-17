#pragma once

#include <Engine/Core/Definitions.h>
#include <Engine/Graphics/DirectXInclude.h>

#include "Fence.h"

class SNOWBITE_API FCommandList
{
public:
	FCommandList(ComPointer<ID3D12Device10> Device, D3D12_COMMAND_LIST_TYPE Type, bool bShouldAutoClose = true,
	             const char* Name = "<unnamed FCommandList>");
	~FCommandList();

	void Reset(ComPointer<ID3D12PipelineState> InitialState = nullptr);
	void Close();
	void Execute(ComPointer<ID3D12CommandQueue> CommandQueue, const std::shared_ptr<FFence>& Fence);

	[[nodiscard]] ComPointer<ID3D12GraphicsCommandList7> GetNativeList() const { return CommandList; }
	[[nodiscard]] ComPointer<ID3D12CommandAllocator> GetCommandAllocator() const { return CommandAllocator; }

private:
	ComPointer<ID3D12GraphicsCommandList7> CommandList;
	ComPointer<ID3D12CommandAllocator> CommandAllocator;
};
