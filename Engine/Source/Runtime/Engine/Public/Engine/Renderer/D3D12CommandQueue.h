#pragma once

#include <Engine/Core/Definitions.h>

#include <Engine/Renderer/D3D12DeviceObject.h>
#include <Engine/Renderer/D3D12Include.h>

#include "D3D12Fence.h"

class SNOWBITE_API FD3D12CommandQueue : public ID3D12DeviceObject
{
public:
	FD3D12CommandQueue(const std::shared_ptr<FD3D12Device>& InDevice);
	~FD3D12CommandQueue() override;

	[[nodiscard]] HRESULT Initialize(D3D12_COMMAND_LIST_TYPE InType);
	void Destroy();

	void ExecuteCommandLists(uint32_t Count, ID3D12CommandList* const* CommandLists) const;

	[[nodiscard]] HRESULT Signal() const;
	[[nodiscard]] HRESULT Signal(const std::shared_ptr<FD3D12Fence>& InFence) const;
	[[nodiscard]] HRESULT WaitForCompletion() const;
	[[nodiscard]] HRESULT WaitForCompletion(const std::shared_ptr<FD3D12Fence>& InFence) const;
	[[nodiscard]] HRESULT SignalAndWaitForCompletion() const;
	[[nodiscard]] HRESULT SignalAndWaitForCompletion(const std::shared_ptr<FD3D12Fence>& InFence) const;

	[[nodiscard]] ComPtr<ID3D12CommandQueue> GetD3D12CommandQueue() const { return CommandQueue; }
	[[nodiscard]] D3D12_COMMAND_LIST_TYPE GetType() const { return Type; }
	[[nodiscard]] std::shared_ptr<FD3D12Fence> GetFence() const { return Fence; }

private:
	ComPtr<ID3D12CommandQueue> CommandQueue;
	D3D12_COMMAND_LIST_TYPE Type;
	std::shared_ptr<FD3D12Fence> Fence;
};
