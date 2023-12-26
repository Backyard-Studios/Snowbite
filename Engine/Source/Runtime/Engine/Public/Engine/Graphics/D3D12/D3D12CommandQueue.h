#pragma once

#include <Engine/Graphics/GraphicsDevice.h>
#include <Engine/Graphics/CommandQueue.h>
#include <Engine/Graphics/D3D12/D3D12Include.h>

class FD3D12CommandQueue : public ICommandQueue
{
public:
	FD3D12CommandQueue();
	~FD3D12CommandQueue() override;

	[[nodiscard]] HRESULT Initialize(std::shared_ptr<IGraphicsDevice> InDevice, ECommandListType InType) override;
	void Destroy() override;

	[[nodiscard]] void* GetNativePointer() const override { return CommandQueue.Get(); }

	[[nodiscard]] ComPtr<ID3D12CommandQueue> GetNativeCommandQueue() const { return CommandQueue; }

private:
	ComPtr<ID3D12CommandQueue> CommandQueue;
};
