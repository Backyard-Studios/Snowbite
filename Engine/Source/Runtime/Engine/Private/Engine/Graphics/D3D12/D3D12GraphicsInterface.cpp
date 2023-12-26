#include "pch.h"

#include <Engine/Graphics/D3D12/D3D12GraphicsInterface.h>

#include "Engine/Graphics/D3D12/D3D12CommandAllocator.h"
#include "Engine/Graphics/D3D12/D3D12CommandList.h"
#include "Engine/Graphics/D3D12/D3D12CommandQueue.h"
#include "Engine/Graphics/D3D12/D3D12Fence.h"
#include "Engine/Graphics/D3D12/D3D12GraphicsAdapter.h"
#include "Engine/Graphics/D3D12/D3D12GraphicsDevice.h"

FD3D12GraphicsInterface::FD3D12GraphicsInterface()
{
}

FD3D12GraphicsInterface::~FD3D12GraphicsInterface()
{
}

HRESULT FD3D12GraphicsInterface::BeginFrame()
{
	const std::shared_ptr<ICommandAllocator> CommandAllocator = Device->GetContext().CommandAllocators[0];
	FD3D12CommandList* CommandList = Device->GetContext().CommandLists[0]->As<FD3D12CommandList>();
	const HRESULT AllocatorResetResult = CommandAllocator->Reset();
	SB_CHECK_RESULT(AllocatorResetResult);
	const HRESULT ListResetResult = CommandList->Reset(CommandAllocator);
	SB_CHECK_RESULT(ListResetResult);
	return S_OK;
}

HRESULT FD3D12GraphicsInterface::EndFrame()
{
	FD3D12CommandList* CommandList = Device->GetContext().CommandLists[0]->As<FD3D12CommandList>();
	const HRESULT ListCloseResult = CommandList->Close();
	SB_CHECK_RESULT(ListCloseResult);
	return S_OK;
}

HRESULT FD3D12GraphicsInterface::Initialize()
{
#if SB_D3D12_ENABLE_DEBUG_LAYER
	{
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&DXGIDebug))))
			DXGIDebug->EnableLeakTrackingForThread();
		ComPtr<ID3D12Debug6> D3D12Debug;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&D3D12Debug))))
		{
			D3D12Debug->EnableDebugLayer();
			D3D12Debug->SetEnableGPUBasedValidation(true);
		}
		D3D12Debug.Reset();
	}
#endif

#if SB_D3D12_ENABLE_DEBUG_LAYER
	uint32_t FactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#else
	uint32_t FactoryFlags = 0;
#endif
	const HRESULT Result = CreateDXGIFactory2(FactoryFlags, IID_PPV_ARGS(&Factory));
	SB_CHECK_RESULT_LOW(Result);

	// Choose the best adapter as default behavior
	// TODO: When we have a settings system, allow the user to choose the adapter
	ComPtr<IDXGIAdapter4> DXGITempAdapter;
	for (uint32_t AdapterIndex = 0; Factory->EnumAdapterByGpuPreference(
		     AdapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
		     IID_PPV_ARGS(&DXGITempAdapter)) != DXGI_ERROR_NOT_FOUND;
	     ++AdapterIndex)
	{
		DXGI_ADAPTER_DESC3 Desc;
		DXGITempAdapter->GetDesc3(&Desc);
		if (Desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;
		// Create a device to see if it supports D3D12
		if (SUCCEEDED(
			D3D12CreateDevice(DXGITempAdapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
		DXGITempAdapter.Reset();
	}
	if (!DXGITempAdapter)
		FPlatform::Fatal("Failed to find a suitable adapter");
	std::shared_ptr<FD3D12GraphicsAdapter> Adapter = std::make_shared<FD3D12GraphicsAdapter>(DXGITempAdapter);
	Device = std::make_shared<FD3D12GraphicsDevice>(Adapter);
	const HRESULT DeviceInitializeResult = Device->Initialize();
	SB_CHECK_RESULT(DeviceInitializeResult);
	SB_SAFE_DESTROY(Adapter);
	const HRESULT ContextInitializeResult = InitializeDeviceContext(Device);
	SB_CHECK_RESULT(ContextInitializeResult);
	return S_OK;
}

void FD3D12GraphicsInterface::Destroy()
{
	SB_SAFE_DESTROY(Device);
	Factory.Reset();
#if SB_D3D12_ENABLE_DEBUG_LAYER
	if (DXGIDebug)
	{
		OutputDebugString(TEXT("Live DXGI objects:\n"));
		DXGIDebug->ReportLiveObjects(DXGI_DEBUG_ALL,
		                             static_cast<DXGI_DEBUG_RLO_FLAGS>(DXGI_DEBUG_RLO_DETAIL |
			                             DXGI_DEBUG_RLO_IGNORE_INTERNAL));
		DXGIDebug.Reset();
	}
#endif
}

HRESULT FD3D12GraphicsInterface::InitializeDeviceContext(const std::shared_ptr<IGraphicsDevice>& InDevice) const
{
	FGraphicsDeviceContext Context = InDevice->GetContext();
	Context.CommandQueue = std::make_shared<FD3D12CommandQueue>();
	const HRESULT QueueInitializeResult = Context.CommandQueue->Initialize(InDevice, ECommandListType::Graphics);
	SB_CHECK_RESULT(QueueInitializeResult);
	Context.CommandAllocators.resize(Context.BufferCount);
	Context.CommandLists.resize(Context.BufferCount);
	Context.Fences.resize(Context.BufferCount);
	for (uint32_t Index = 0; Index < Context.BufferCount; ++Index)
	{
		Context.CommandAllocators[Index] = std::make_shared<FD3D12CommandAllocator>();
		const HRESULT AllocatorInitializeResult = Context.CommandAllocators[Index]->Initialize(
			InDevice, ECommandListType::Graphics);
		SB_CHECK_RESULT(AllocatorInitializeResult);
		Context.CommandLists[Index] = std::make_shared<FD3D12CommandList>();
		const HRESULT ListInitializeResult = Context.CommandLists[Index]->Initialize(
			InDevice, ECommandListType::Graphics);
		SB_CHECK_RESULT(ListInitializeResult);
		Context.Fences[Index] = std::make_shared<FD3D12Fence>();
		const HRESULT FenceInitializeResult = Context.Fences[Index]->Initialize(InDevice);
		SB_CHECK_RESULT(FenceInitializeResult);
	}
	return S_OK;
}
