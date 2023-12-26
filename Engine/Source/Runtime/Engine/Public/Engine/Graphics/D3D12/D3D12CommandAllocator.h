#pragma once

#include <Engine/Graphics/GraphicsDevice.h>
#include <Engine/Graphics/CommandAllocator.h>
#include <Engine/Graphics/D3D12/D3D12Include.h>

#include "Engine/Graphics/CommandListType.h"

class FD3D12CommandAllocator : public ICommandAllocator
{
public:
	FD3D12CommandAllocator();
	~FD3D12CommandAllocator() override;

	[[nodiscard]] HRESULT Initialize(std::shared_ptr<IGraphicsDevice> InDevice, ECommandListType InType) override;
	void Destroy() override;

	[[nodiscard]] HRESULT Reset() override;

	[[nodiscard]] void* GetNativePointer() override { return CommandAllocator.Get(); }

	[[nodiscard]] ComPtr<ID3D12CommandAllocator> GetNativeCommandAllocator() const { return CommandAllocator; }

private:
	ComPtr<ID3D12CommandAllocator> CommandAllocator;
};
