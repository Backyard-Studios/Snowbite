#include "pch.h"

#include <Engine/Graphics/D3D12/D3D12CommandQueue.h>

#include "Engine/Graphics/D3D12/D3D12CommandListType.h"
#include "Engine/Graphics/D3D12/D3D12GraphicsDevice.h"

FD3D12CommandQueue::FD3D12CommandQueue()
{
}

FD3D12CommandQueue::~FD3D12CommandQueue()
{
}

HRESULT FD3D12CommandQueue::Initialize(const std::shared_ptr<IGraphicsDevice> InDevice, const ECommandListType InType)
{
	const ComPtr<ID3D12Device13> Device = InDevice->As<FD3D12GraphicsDevice>()->GetNativeDevice();
	D3D12_COMMAND_QUEUE_DESC Desc;
	Desc.Type = ConvertCommandListType(InType);
	Desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
	Desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	Desc.NodeMask = 0;
	const HRESULT Result = Device->CreateCommandQueue(&Desc, IID_PPV_ARGS(&CommandQueue));
	SB_CHECK_RESULT_LOW(Result);
	return S_OK;
}

void FD3D12CommandQueue::Destroy()
{
	CommandQueue.Reset();
}
