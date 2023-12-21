#pragma once

#include <Engine/Core/Definitions.h>

#include <Engine/Graphics/D3D12/D3D12Include.h>
#include <Engine/Graphics/D3D12/D3D12Fence.h>

class SNOWBITE_API FD3D12CommandList
{
public:
	FD3D12CommandList(const ComPointer<ID3D12Device10>& InDevice, D3D12_COMMAND_LIST_TYPE InType,
	                  const char* InName = "<unnamed D3D12CommandList>");
	~FD3D12CommandList();

	HRESULT Initialize();
	void Destroy();

	HRESULT Reset(ComPointer<ID3D12PipelineState> InitialState = nullptr);
	HRESULT Close();
	HRESULT Execute(ComPointer<ID3D12CommandQueue> CommandQueue, const std::shared_ptr<FD3D12Fence>& Fence);

	[[nodiscard]] ComPointer<ID3D12GraphicsCommandList7> GetNativeList() const { return CommandList; }
	[[nodiscard]] ComPointer<ID3D12CommandAllocator> GetCommandAllocator() const { return CommandAllocator; }

private:
	ComPointer<ID3D12Device10> Device;
	D3D12_COMMAND_LIST_TYPE Type;
	const char* Name;

	ComPointer<ID3D12CommandAllocator> CommandAllocator;
	ComPointer<ID3D12GraphicsCommandList7> CommandList;
};
