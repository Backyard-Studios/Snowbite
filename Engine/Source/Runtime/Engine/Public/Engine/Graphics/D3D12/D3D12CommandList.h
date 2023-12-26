#pragma once

#include <Engine/Graphics/GraphicsDevice.h>
#include <Engine/Graphics/CommandList.h>
#include <Engine/Graphics/D3D12/D3D12Include.h>

class FD3D12CommandList : public ICommandList
{
public:
	FD3D12CommandList();
	~FD3D12CommandList() override;

	[[nodiscard]] HRESULT Initialize(std::shared_ptr<IGraphicsDevice> InDevice, ECommandListType InType) override;
	void Destroy() override;

	[[nodiscard]] HRESULT Reset(std::shared_ptr<ICommandAllocator> InCommandAllocator) override;
	[[nodiscard]] HRESULT Close() override;

	[[nodiscard]] void* GetNativePointer() override { return CommandList.Get(); }

	[[nodiscard]] ComPtr<ID3D12GraphicsCommandList9> GetNativeCommandList() const { return CommandList; }

private:
	ComPtr<ID3D12GraphicsCommandList9> CommandList;
};
