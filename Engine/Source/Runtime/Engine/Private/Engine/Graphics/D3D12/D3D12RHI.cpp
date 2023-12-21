﻿#include "pch.h"

#include <Engine/Graphics/D3D12/D3D12RHI.h>

#include "Engine/Graphics/D3D12/D3D12Utils.h"

FD3D12RHI::FD3D12RHI(const FD3D12RHISettings& InSettings)
	: Settings(InSettings), BufferCount(static_cast<uint32_t>(InSettings.BufferingMode))
{
}

FD3D12RHI::~FD3D12RHI() = default;

HRESULT FD3D12RHI::Initialize()
{
	if (Settings.bEnableDebugLayer)
	{
		const HRESULT GetDXGIDebugInterfaceResult = DXGIGetDebugInterface1(0, IID_PPV_ARGS(&DXGIDebug));
		if (SUCCEEDED(GetDXGIDebugInterfaceResult))
		{
			DXGIDebug->EnableLeakTrackingForThread();
			SB_LOG_DEBUG("Enabled dxgi leak tracking");
		}
		else
			SB_LOG_WARN("Failed to get dxgi debug interface");

		ComPointer<ID3D12Debug6> D3D12Debug;
		const HRESULT GetD3D12DebugInterfaceResult = D3D12GetDebugInterface(IID_PPV_ARGS(&D3D12Debug));
		if (SUCCEEDED(GetD3D12DebugInterfaceResult))
		{
			D3D12Debug->EnableDebugLayer();
			D3D12Debug->SetEnableGPUBasedValidation(Settings.bEnableGPUValidation);
			SB_LOG_DEBUG("Enabled d3d12 debug layer [GPU Validation: {}]",
			             Settings.bEnableGPUValidation ? "true" : "false");
		}
		else
			SB_LOG_WARN("Failed to get d3d12 debug interface");
		D3D12Debug->Release();
	}
	const uint32_t FactoryFlags = Settings.bEnableDebugLayer ? DXGI_CREATE_FACTORY_DEBUG : 0;
	const HRESULT FactoryCreateResult = CreateDXGIFactory2(FactoryFlags, IID_PPV_ARGS(&Factory));
	SB_D3D_ASSERT_RETURN(FactoryCreateResult, "Unable to create dxgi factory");

	Device = std::make_shared<FD3D12Device>(Factory, FD3D12DeviceDesc());
	const HRESULT DeviceInitializeResult = Device->Initialize();
	SB_D3D_ASSERT_RETURN(DeviceInitializeResult, "Unable to initialize d3d12 device");

	const HRESULT CreateCommandQueueResult = Device->CreateCommandQueue(
		GraphicsCommandQueue, D3D12_COMMAND_LIST_TYPE_DIRECT, "GraphicsCommandQueue");
	SB_D3D_ASSERT_RETURN(CreateCommandQueueResult, "Unable to create graphics command queue");

	const HRESULT CreateSwapChainResult = CreateSwapChain();
	SB_D3D_ASSERT_RETURN(CreateSwapChainResult, "Unable to create swap chain");

	const HRESULT InitializePerFrameDataResult = InitializePerFrameData();
	SB_D3D_ASSERT_RETURN(InitializePerFrameDataResult, "Unable to initialize per frame data");

	Settings.Window->SetTitle(std::format("Snowbite | {} ({})", GetName(),
	                                      D3D12Utils::ShaderModelToMajorString(Device->GetShaderModel())).c_str());
	return S_OK;
}

void FD3D12RHI::Shutdown()
{
	FlushFrames(BufferCount);
	DestroyPerFrameData();
	SwapChain->Destroy();
	SB_SAFE_RESET(SwapChain);
	GraphicsCommandQueue.Release();
	Device->Destroy();
	SB_SAFE_RESET(Device);
	Factory.Release();
	if (Settings.bEnableDebugLayer && DXGIDebug)
	{
		if (DXGIDebug->IsLeakTrackingEnabledForThread())
			SB_LOG_DEBUG("Reporting live objects. Look into the debug output for more information");
		uint32_t Flags = DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL;
		const HRESULT ReportObjectsResult = DXGIDebug->ReportLiveObjects(
			DXGI_DEBUG_ALL, static_cast<DXGI_DEBUG_RLO_FLAGS>(Flags));
		if (FAILED(ReportObjectsResult))
			SB_LOG_WARN("Failed to report live objects");
		DXGIDebug->Release();
	}
}

HRESULT FD3D12RHI::PrepareNextFrame()
{
	BufferIndex = SwapChain->GetFrameIndex();
	const FFrameData FrameData = PerFrameData[BufferIndex];
	const std::shared_ptr<FD3D12CommandList> CommandListContainer = FrameData.CommandList;
	ComPointer<ID3D12GraphicsCommandList7> CommandList = CommandListContainer->GetNativeList();
	const HRESULT ResetResult = CommandListContainer->Reset();
	SB_D3D_FAILED_RETURN(ResetResult);

	// Transition back buffer to render target
	D3D12_RESOURCE_BARRIER Barrier;
	Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	Barrier.Transition.pResource = SwapChain->GetBackBuffer().Get();
	Barrier.Transition.Subresource = 0;
	Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	CommandList->ResourceBarrier(1, &Barrier);

	const D3D12_CPU_DESCRIPTOR_HANDLE BackBufferDescriptor = SwapChain->GetBackBufferDescriptor();
	CommandList->OMSetRenderTargets(1, &BackBufferDescriptor, FALSE, nullptr);
	const float ClearColorArray[] = {
		BackBufferClearColor.Red, BackBufferClearColor.Green, BackBufferClearColor.Blue, BackBufferClearColor.Alpha
	};
	CommandList->ClearRenderTargetView(BackBufferDescriptor, ClearColorArray, 0, nullptr);
	return S_OK;
}

HRESULT FD3D12RHI::WaitForFrame(const uint32_t Index)
{
	const HRESULT SignalResult = PerFrameData[Index].Fence->Signal(GraphicsCommandQueue);
	SB_D3D_FAILED_RETURN(SignalResult);
	const HRESULT WaitResult = PerFrameData[Index].Fence->WaitOnCPU();
	SB_D3D_FAILED_RETURN(WaitResult);
	return S_OK;
}

HRESULT FD3D12RHI::FlushFrames(uint32_t Count)
{
	if (Count > BufferCount)
	{
		Count = BufferCount;
		SB_LOG_WARN("FLush feames count is greater than buffer count. Clamping to buffer count");
	}
	for (uint32_t Index = 0; Index < Count; ++Index)
	{
		const HRESULT WaitResult = WaitForFrame(Index);
		SB_D3D_FAILED_RETURN(WaitResult);
	}
	return S_OK;
}

void FD3D12RHI::SetBackBufferClearColor(const FClearColor& ClearColor)
{
	BackBufferClearColor = ClearColor;
}

HRESULT FD3D12RHI::PresentFrame()
{
	const FFrameData FrameData = PerFrameData[BufferIndex];
	const std::shared_ptr<FD3D12CommandList> CommandListContainer = FrameData.CommandList;
	ComPointer<ID3D12GraphicsCommandList7> CommandList = CommandListContainer->GetNativeList();

	// Transition back buffer to present
	D3D12_RESOURCE_BARRIER Barrier;
	Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	Barrier.Transition.pResource = SwapChain->GetBackBuffer().Get();
	Barrier.Transition.Subresource = 0;
	Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	CommandList->ResourceBarrier(1, &Barrier);

	const std::shared_ptr<FD3D12Fence> Fence = FrameData.Fence;
	const HRESULT ExecuteResult = CommandListContainer->Execute(GraphicsCommandQueue, Fence);
	SB_D3D_FAILED_RETURN(ExecuteResult);
	const HRESULT PresentResult = SwapChain->Present(true);
	SB_D3D_FAILED_RETURN(PresentResult);
	return S_OK;
}

uint32_t FD3D12RHI::GetBufferCount() const
{
	return BufferCount;
}

uint32_t FD3D12RHI::GetBufferIndex() const
{
	return BufferIndex;
}

HRESULT FD3D12RHI::CreateSwapChain()
{
	FD3D12SwapChainDesc SwapChainDesc;
	SwapChainDesc.Width = Settings.Window->GetWidth();
	SwapChainDesc.Height = Settings.Window->GetHeight();
	SwapChainDesc.BufferCount = BufferCount;
	SwapChainDesc.Window = Settings.Window;
	SwapChainDesc.Format = Device->GetRenderTargetViewFormat();
	SwapChain = std::make_shared<FD3D12SwapChain>(Factory, Device->GetNativeDevice(), GraphicsCommandQueue,
	                                              SwapChainDesc);
	const HRESULT SwapChainInitializeResult = SwapChain->Initialize();
	SB_D3D_FAILED_RETURN(SwapChainInitializeResult);
	return S_OK;
}

HRESULT FD3D12RHI::InitializePerFrameData()
{
	PerFrameData.resize(BufferCount);
	for (uint32_t Index = 0; Index < BufferCount; ++Index)
	{
		PerFrameData[Index].Fence = Device->CreateFence(("FrameFence " + std::to_string(Index)).c_str());
		const HRESULT InitializeFenceResult = PerFrameData[Index].Fence->Initialize();
		SB_D3D_ASSERT_RETURN(InitializeFenceResult, "Unable to initialize fence for frame {}", Index);

		PerFrameData[Index].CommandList = Device->CreateCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT,
		                                                            ("FrameCommandList " + std::to_string(Index)).
		                                                            c_str());
		const HRESULT InitializeCommandListResult = PerFrameData[Index].CommandList->Initialize();
		SB_D3D_ASSERT_RETURN(InitializeCommandListResult, "Unable to initialize command list for frame {}", Index);
		PerFrameData[Index].CommandList->Close();
	}
	return S_OK;
}

void FD3D12RHI::DestroyPerFrameData()
{
	for (FFrameData& FrameData : PerFrameData)
	{
		FrameData.Fence->Destroy();
		FrameData.Fence.reset();

		FrameData.CommandList->Destroy();
		FrameData.CommandList.reset();
	}
	PerFrameData.clear();
}
