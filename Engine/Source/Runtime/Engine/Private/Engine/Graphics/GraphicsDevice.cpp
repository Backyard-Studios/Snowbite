#include "pch.h"

#include <codecvt>
#include <Engine/Graphics/GraphicsDevice.h>

// ReSharper disable CppNonInlineVariableDefinitionInHeaderFile
extern "C" {
// http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;

//  https://gpuopen.com/amdpowerxpressrequesthighperformance/
_declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001;
}

// ReSharper enable CppNonInlineVariableDefinitionInHeaderFile

FGraphicsDevice::FGraphicsDevice(const FGraphicsDeviceSettings& InSettings)
	: Settings(InSettings)
{
	const uint32_t FactoryCreationFlags = Settings.bEnableDebugLayer ? DXGI_CREATE_FACTORY_DEBUG : 0;
	const HRESULT CreateFactoryResult = CreateDXGIFactory2(FactoryCreationFlags, IID_PPV_ARGS(&Factory));
	SB_D3D_ASSERT(CreateFactoryResult, "Failed to create DXGI factory");
	if (Settings.bEnableDebugLayer)
	{
		const HRESULT EnableDebugLayerResult = D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController));
		SB_D3D_ASSERT(EnableDebugLayerResult, "Failed to enable debug layer");
		DebugController->EnableDebugLayer();
		DebugController->SetEnableGPUBasedValidation(Settings.bEnableGPUValidation);

		const HRESULT EnableDXGIDebugResult = DXGIGetDebugInterface1(0, IID_PPV_ARGS(&DXGIDebugController));
		SB_D3D_ASSERT(EnableDXGIDebugResult, "Failed to enable DXGI debug");
		DXGIDebugController->EnableLeakTrackingForThread();
		SB_LOG_INFO("D3D12 debug layer enabled");
	}
	ComPointer<IDXGIAdapter1> Adapter;
	DXGI_ADAPTER_DESC1 AdapterDesc;
	uint32_t AdapterIndex = 0;
	bool bAdapterFound = false;
	while (Factory->EnumAdapterByGpuPreference(AdapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
	                                           IID_PPV_ARGS(&Adapter)) != DXGI_ERROR_NOT_FOUND)
	{
		Adapter->GetDesc1(&AdapterDesc);
		if (AdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			AdapterIndex++;
			continue;
		}
		const HRESULT DeviceCreateResult = D3D12CreateDevice(Adapter, Settings.FeatureLevel, _uuidof(ID3D12Device),
		                                                     nullptr);
		if (SUCCEEDED(DeviceCreateResult))
		{
			bAdapterFound = true;
			break;
		}
		Adapter.Release();
		AdapterIndex++;
	}
	SB_ASSERT_CRITICAL(bAdapterFound, E_FAIL, "Failed to find suitable adapter");
	char AdapterNameBuffer[128];
	WideCharToMultiByte(CP_ACP, 0, AdapterDesc.Description, -1, AdapterNameBuffer, 128, nullptr, nullptr);
	std::string AdapterName(AdapterNameBuffer);
	SB_LOG_INFO("Using '{}' adapter", AdapterName);
	SB_LOG_INFO("\t- Dedicated video memory: {} MB", AdapterDesc.DedicatedVideoMemory / 1024 / 1024);
	SB_LOG_INFO("\t- Shared system memory: {} MB", AdapterDesc.SharedSystemMemory / 1024 / 1024);

	const HRESULT DeviceCreateResult = D3D12CreateDevice(Adapter, Settings.FeatureLevel, IID_PPV_ARGS(&Device));
	SB_D3D_ASSERT(DeviceCreateResult, "Failed to create D3D12 device");
	Adapter.Release();

	D3D12_COMMAND_QUEUE_DESC CommandQueueDesc;
	CommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	CommandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
	CommandQueueDesc.NodeMask = 0;
	CommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	const HRESULT CommandQueueCreateResult = Device->CreateCommandQueue(&CommandQueueDesc, IID_PPV_ARGS(&CommandQueue));
	SB_D3D_ASSERT(CommandQueueCreateResult, "Failed to create command queue");

	uint32_t BufferCount = static_cast<uint32_t>(Settings.BufferingMode);
	CommandAllocators.resize(BufferCount);
	CommandLists.resize(BufferCount);
	Fences.resize(BufferCount);
	FenceValues.resize(BufferCount);
	for (uint32_t Index = 0; Index < BufferCount; ++Index)
	{
		const HRESULT CommandAllocatorCreateResult = Device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocators[Index]));
		SB_D3D_ASSERT(CommandAllocatorCreateResult, "Failed to create command allocator");

		const HRESULT CommandListCreateResult = Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		                                                                  CommandAllocators[Index].Get(), nullptr,
		                                                                  IID_PPV_ARGS(&CommandLists[Index]));
		SB_D3D_ASSERT(CommandListCreateResult, "Failed to create command list");
		CommandLists[Index]->Close();

		const HRESULT FenceCreateResult = Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fences[Index]));
		SB_D3D_ASSERT(FenceCreateResult, "Failed to create fence");
		FenceValues[Index] = 0;
	}
	FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	SB_ASSERT_CRITICAL(FenceEvent != nullptr, E_FAIL, "Failed to create fence event");

	D3D12_DESCRIPTOR_HEAP_DESC RtvDescriptorHeapDesc;
	RtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	RtvDescriptorHeapDesc.NumDescriptors = 3;
	RtvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	RtvDescriptorHeapDesc.NodeMask = 0;
	const HRESULT DescriptorHeapCreateResult = Device->CreateDescriptorHeap(
		&RtvDescriptorHeapDesc, IID_PPV_ARGS(&RtvDescriptorHeap));
	SB_D3D_ASSERT(DescriptorHeapCreateResult, "Failed to create descriptor heap");
	RtvDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	FSwapChainDesc SwapChainDesc;
	SwapChainDesc.Width = 1280;
	SwapChainDesc.Height = 720;
	SwapChainDesc.BufferCount = static_cast<uint32_t>(Settings.BufferingMode);
	SwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChainDesc.Window = Settings.Window;
	SwapChain = std::make_shared<FSwapChain>(this, SwapChainDesc);
}

FGraphicsDevice::~FGraphicsDevice()
{
	Flush(static_cast<uint32_t>(Settings.BufferingMode));
	if (FenceEvent)
		CloseHandle(FenceEvent);
	FenceValues.clear();
	for (ComPointer<ID3D12Fence1>& Fence : Fences)
		Fence.Release();
	Fences.clear();
	for (ComPointer<ID3D12GraphicsCommandList4>& CommandList : CommandLists)
		CommandList.Release();
	CommandLists.clear();
	for (ComPointer<ID3D12CommandAllocator>& CommandAllocator : CommandAllocators)
		CommandAllocator.Release();
	CommandAllocators.clear();
	SB_SAFE_RESET(SwapChain);
	RtvDescriptorHeap.Release();
	CommandQueue.Release();
	Device.Release();
	if (Settings.bEnableDebugLayer)
	{
		DXGIDebugController->ReportLiveObjects(DXGI_DEBUG_ALL,
		                                       static_cast<DXGI_DEBUG_RLO_FLAGS>(DXGI_DEBUG_RLO_DETAIL |
			                                       DXGI_DEBUG_RLO_IGNORE_INTERNAL));
		DXGIDebugController.Release();
		DebugController.Release();
	}
	Factory.Release();
}

void FGraphicsDevice::SignalAndWait()
{
	ComPointer<ID3D12Fence1> Fence = Fences[SwapChain->GetFrameIndex()];
	FenceValues[SwapChain->GetFrameIndex()]++;
	CommandQueue->Signal(Fence, FenceValues[SwapChain->GetFrameIndex()]);
	const HRESULT WaitResult = Fence->SetEventOnCompletion(FenceValues[SwapChain->GetFrameIndex()], FenceEvent);
	const DWORD WaitForSingleObjectResult = WaitForSingleObject(FenceEvent, 20000);
	SB_ASSERT_CRITICAL(WaitForSingleObjectResult == WAIT_OBJECT_0, E_FAIL, "Failed to wait for fence");
	SB_D3D_ASSERT(WaitResult, "Failed to set event on completion");
}

void FGraphicsDevice::Flush(const uint32_t Count)
{
	for (uint32_t Index = 0; Index < Count; ++Index)
		SignalAndWait();
}

void FGraphicsDevice::Resize(const uint32_t InWidth, const uint32_t InHeight)
{
	Flush(static_cast<uint32_t>(Settings.BufferingMode));
	SwapChain->Resize(InWidth, InHeight);
}

void FGraphicsDevice::BeginFrame(const FClearColor& ClearColor) const
{
	ComPointer<ID3D12CommandAllocator> CommandAllocator = CommandAllocators[SwapChain->GetFrameIndex()];
	const HRESULT ResetCommandAllocatorResult = CommandAllocator->Reset();
	SB_D3D_ASSERT(ResetCommandAllocatorResult, "Failed to reset command allocator");

	ComPointer<ID3D12GraphicsCommandList4> CommandList = CommandLists[SwapChain->GetFrameIndex()];
	const HRESULT ResetCommandListResult = CommandList->Reset(CommandAllocator.Get(), nullptr);
	SB_D3D_ASSERT(ResetCommandListResult, "Failed to reset command list");

	D3D12_RESOURCE_BARRIER Barrier;
	Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	Barrier.Transition.pResource = SwapChain->GetBackBuffer().Get();
	Barrier.Transition.Subresource = 0;
	Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	CommandList->ResourceBarrier(1, &Barrier);

	const float ClearColorArray[] = {ClearColor.Red, ClearColor.Green, ClearColor.Blue, ClearColor.Alpha};
	const D3D12_CPU_DESCRIPTOR_HANDLE BackBufferDescriptor = SwapChain->GetBackBufferDescriptor();
	CommandList->ClearRenderTargetView(BackBufferDescriptor, ClearColorArray, 0, nullptr);
	CommandList->OMSetRenderTargets(1, &BackBufferDescriptor, FALSE, nullptr);
}

void FGraphicsDevice::EndFrame()
{
	ComPointer<ID3D12GraphicsCommandList4> CommandList = CommandLists[SwapChain->GetFrameIndex()];

	D3D12_RESOURCE_BARRIER Barrier;
	Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	Barrier.Transition.pResource = SwapChain->GetBackBuffer().Get();
	Barrier.Transition.Subresource = 0;
	Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	CommandList->ResourceBarrier(1, &Barrier);

	const HRESULT CloseCommandListResult = CommandList->Close();
	SB_D3D_ASSERT(CloseCommandListResult, "Failed to close command list");

	ID3D12CommandList* ExecuteCommandLists[] = {CommandList.Get()};
	CommandQueue->ExecuteCommandLists(_countof(ExecuteCommandLists), ExecuteCommandLists);
	SignalAndWait();
	SwapChain->Present();
}
