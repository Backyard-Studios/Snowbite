#include "pch.h"

#include <Engine/Graphics/D3D12/D3D12GraphicsDevice.h>

#include "Engine/Graphics/D3D12/D3D12CommandAllocator.h"
#include "Engine/Graphics/D3D12/D3D12CommandList.h"
#include "Engine/Graphics/D3D12/D3D12CommandQueue.h"

FD3D12GraphicsDevice::FD3D12GraphicsDevice(const std::shared_ptr<FD3D12GraphicsAdapter>& InAdapter)
	: Adapter(InAdapter)
{
}

FD3D12GraphicsDevice::~FD3D12GraphicsDevice()
{
}

HRESULT FD3D12GraphicsDevice::Initialize()
{
	const HRESULT CreateResult = D3D12CreateDevice(Adapter->GetNativeAdapter().Get(),
	                                               Adapter->GetMaxSupportedFeatureLevel(),
	                                               IID_PPV_ARGS(&Device));
	SB_CHECK_RESULT_LOW(CreateResult);
	return S_OK;
}

void FD3D12GraphicsDevice::Destroy()
{
	for (std::shared_ptr<IFence>& Fence : Context.Fences)
	{
		Fence->Destroy();
		Fence.reset();
	}
	Context.Fences.clear();
	for (std::shared_ptr<ICommandList>& CommandList : Context.CommandLists)
	{
		CommandList->Destroy();
		CommandList.reset();
	}
	Context.CommandLists.clear();
	for (std::shared_ptr<ICommandAllocator>& CommandAllocator : Context.CommandAllocators)
	{
		CommandAllocator->Destroy();
		CommandAllocator.reset();
	}
	Context.CommandAllocators.clear();
	SB_SAFE_DESTROY(Context.CommandQueue);
	Device.Reset();
	Adapter->Destroy();
	SB_SAFE_RESET(Adapter);
}
