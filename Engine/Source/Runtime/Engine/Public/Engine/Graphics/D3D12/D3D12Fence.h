#pragma once

#include <Engine/Graphics/Fence.h>

class FD3D12Fence : public IFence
{
public:
	FD3D12Fence();
	~FD3D12Fence() override = default;

	[[nodiscard]] HRESULT Initialize(std::shared_ptr<IGraphicsDevice> InDevice) override;
	void Destroy() override;

	[[nodiscard]] HRESULT Signal(std::shared_ptr<ICommandQueue> InQueue) override;
	[[nodiscard]] HRESULT Wait() override;

	[[nodiscard]] void* GetNativePointer() const override { return Fence.Get(); }

	[[nodiscard]] ComPtr<ID3D12Fence1> GetFence() const { return Fence; }

private:
	ComPtr<ID3D12Fence1> Fence;
	HANDLE FenceEvent = nullptr;
};
