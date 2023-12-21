#include "pch.h"

#include <Engine/Graphics/D3D12/D3D12RHI.h>

#include "backends/imgui_impl_dx12.h"
#include "backends/imgui_impl_win32.h"
#include "Engine/Graphics/D3D12/D3D12Utils.h"

FD3D12RHI::FD3D12RHI(const FD3D12RHISettings& InSettings)
	: Settings(InSettings), BufferCount(static_cast<uint32_t>(InSettings.BufferingMode))
{
}

FD3D12RHI::~FD3D12RHI() = default;

HRESULT FD3D12RHI::Initialize()
{
	Width = Settings.Window->GetWidth();
	Height = Settings.Window->GetHeight();
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

	const HRESULT InitializePerFrameDataResult = InitializeFrameContexts();
	SB_D3D_ASSERT_RETURN(InitializePerFrameDataResult, "Unable to initialize per frame data");

	D3D12_DESCRIPTOR_HEAP_DESC DSVDescriptorHeapDesc = {};
	DSVDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DSVDescriptorHeapDesc.NumDescriptors = 1;
	DSVDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	const HRESULT DSVDescriptorHeapCreateResult = Device->GetNativeDevice()->CreateDescriptorHeap(
		&DSVDescriptorHeapDesc, IID_PPV_ARGS(&DSVDescriptorHeap));
	SB_D3D_FAILED_RETURN(DSVDescriptorHeapCreateResult);

	D3D12_DESCRIPTOR_HEAP_DESC SRVDescriptorHeapDesc = {};
	SRVDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	SRVDescriptorHeapDesc.NumDescriptors = 1;
	SRVDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	const HRESULT SRVDescriptorHeapCreateResult = Device->GetNativeDevice()->CreateDescriptorHeap(
		&SRVDescriptorHeapDesc, IID_PPV_ARGS(&SRVDescriptorHeap));
	SB_D3D_ASSERT_RETURN(SRVDescriptorHeapCreateResult, "Unable to create SRV descriptor heap");

	const HRESULT CreateDepthStencilResult = CreateDepthStencil();
	SB_D3D_ASSERT_RETURN(CreateDepthStencilResult, "Unable to create depth stencil");

	Settings.Window->SetTitle(std::format("Snowbite | {} ({})", GetName(),
	                                      D3D12Utils::ShaderModelToMajorString(Device->GetShaderModel())).c_str());

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& IO = ImGui::GetIO();
	IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	IO.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	IO.DisplaySize = ImVec2(static_cast<float>(Width), static_cast<float>(Height));
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(Settings.Window->GetHandle());
	ImGui_ImplDX12_Init(Device->GetNativeDevice(), BufferCount, Device->GetRenderTargetViewFormat(), SRVDescriptorHeap,
	                    SRVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
	                    SRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	return S_OK;
}

void FD3D12RHI::Shutdown()
{
	FlushFrames(BufferCount);
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	DestroyDepthStencil();
	SRVDescriptorHeap.Release();
	DSVDescriptorHeap.Release();
	DestroyFrameContexts();
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
	const FFrameContext FrameContext = FrameContexts[BufferIndex];
	const std::shared_ptr<FD3D12CommandList> CommandListContainer = FrameContext.CommandList;
	ComPointer<ID3D12GraphicsCommandList7> CommandList = CommandListContainer->GetNativeList();
	const HRESULT ResetResult = CommandListContainer->Reset();
	SB_D3D_FAILED_RETURN(ResetResult);

	D3D12_RESOURCE_BARRIER Barrier;
	Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	Barrier.Transition.pResource = SwapChain->GetBackBuffer().Get();
	Barrier.Transition.Subresource = 0;
	Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	CommandList->ResourceBarrier(1, &Barrier);

	const D3D12_CPU_DESCRIPTOR_HANDLE BackBufferDescriptor = SwapChain->GetBackBufferDescriptor();
	const D3D12_CPU_DESCRIPTOR_HANDLE DSVDescriptor = DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	CommandList->OMSetRenderTargets(1, &BackBufferDescriptor, FALSE, &DSVDescriptor);
	const float ClearColorArray[] = {
		BackBufferClearColor.Red, BackBufferClearColor.Green, BackBufferClearColor.Blue, BackBufferClearColor.Alpha
	};
	CommandList->ClearRenderTargetView(BackBufferDescriptor, ClearColorArray, 0, nullptr);
	CommandList->ClearDepthStencilView(DSVDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	return S_OK;
}

HRESULT FD3D12RHI::PresentFrame(const bool bShouldVSync)
{
	const FFrameContext FrameContext = FrameContexts[BufferIndex];
	const std::shared_ptr<FD3D12CommandList> CommandListContainer = FrameContext.CommandList;
	ComPointer<ID3D12GraphicsCommandList7> CommandList = CommandListContainer->GetNativeList();

	D3D12_RESOURCE_BARRIER Barrier;
	Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	Barrier.Transition.pResource = SwapChain->GetBackBuffer().Get();
	Barrier.Transition.Subresource = 0;
	Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	CommandList->ResourceBarrier(1, &Barrier);

	const std::shared_ptr<FD3D12Fence> Fence = FrameContext.Fence;
	const HRESULT ExecuteResult = CommandListContainer->Execute(GraphicsCommandQueue, Fence);
	SB_D3D_FAILED_RETURN(ExecuteResult);
	const HRESULT PresentResult = SwapChain->Present(bShouldVSync);
	SB_D3D_FAILED_RETURN(PresentResult);
	return S_OK;
}

HRESULT FD3D12RHI::WaitForFrame(const uint32_t Index)
{
	const HRESULT SignalResult = FrameContexts[Index].Fence->Signal(GraphicsCommandQueue);
	SB_D3D_FAILED_RETURN(SignalResult);
	const HRESULT WaitResult = FrameContexts[Index].Fence->WaitOnCPU();
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

void FD3D12RHI::BeginUIFrame()
{
	ID3D12DescriptorHeap* SrvDescriptorHeaps[] = {
		SRVDescriptorHeap.Get(),
	};
	FrameContexts[BufferIndex].CommandList->GetNativeList()->SetDescriptorHeaps(
		_countof(SrvDescriptorHeaps), SrvDescriptorHeaps);
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void FD3D12RHI::EndUIFrame()
{
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), FrameContexts[BufferIndex].CommandList->GetNativeList());
}

HRESULT FD3D12RHI::Resize(const uint32_t InWidth, const uint32_t InHeight)
{
	FlushFrames(BufferCount);
	Width = InWidth;
	Height = InHeight;
	const HRESULT ResizeResult = SwapChain->Resize(Width, Height);
	SB_D3D_FAILED_RETURN(ResizeResult);
	DestroyDepthStencil();
	const HRESULT CreateDepthStencilResult = CreateDepthStencil();
	SB_D3D_FAILED_RETURN(CreateDepthStencilResult);
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(static_cast<float>(Width), static_cast<float>(Height));
	return S_OK;
}

void FD3D12RHI::SetBackBufferClearColor(const FClearColor& ClearColor)
{
	BackBufferClearColor = ClearColor;
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
	SwapChainDesc.Width = Width;
	SwapChainDesc.Height = Height;
	SwapChainDesc.BufferCount = BufferCount;
	SwapChainDesc.Window = Settings.Window;
	SwapChainDesc.Format = Device->GetRenderTargetViewFormat();
	SwapChain = std::make_shared<FD3D12SwapChain>(Factory, Device->GetNativeDevice(), GraphicsCommandQueue,
	                                              SwapChainDesc);
	const HRESULT SwapChainInitializeResult = SwapChain->Initialize();
	SB_D3D_FAILED_RETURN(SwapChainInitializeResult);
	return S_OK;
}

HRESULT FD3D12RHI::InitializeFrameContexts()
{
	FrameContexts.resize(BufferCount);
	for (uint32_t Index = 0; Index < BufferCount; ++Index)
	{
		FrameContexts[Index].Fence = Device->CreateFence(("FrameFence " + std::to_string(Index)).c_str());
		const HRESULT InitializeFenceResult = FrameContexts[Index].Fence->Initialize();
		SB_D3D_ASSERT_RETURN(InitializeFenceResult, "Unable to initialize fence for frame {}", Index);

		FrameContexts[Index].CommandList = Device->CreateCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT,
		                                                             ("FrameCommandList " + std::to_string(Index)).
		                                                             c_str());
		const HRESULT InitializeCommandListResult = FrameContexts[Index].CommandList->Initialize();
		SB_D3D_ASSERT_RETURN(InitializeCommandListResult, "Unable to initialize command list for frame {}", Index);
		FrameContexts[Index].CommandList->Close();
	}
	return S_OK;
}

void FD3D12RHI::DestroyFrameContexts()
{
	for (FFrameContext& FrameContext : FrameContexts)
	{
		FrameContext.Fence->Destroy();
		FrameContext.Fence.reset();

		FrameContext.CommandList->Destroy();
		FrameContext.CommandList.reset();
	}
	FrameContexts.clear();
}

HRESULT FD3D12RHI::CreateDepthStencil()
{
	DepthStencil = std::make_shared<FD3D12DepthStencil>(Device->GetNativeDevice(), Device->GetAllocator(),
	                                                    DSVDescriptorHeap);
	const HRESULT CreateResult = DepthStencil->Initialize(Device->GetDepthStencilViewFormat(), Width, Height);
	SB_D3D_FAILED_RETURN(CreateResult);
	return S_OK;
}

void FD3D12RHI::DestroyDepthStencil()
{
	DepthStencil->Destroy();
	SB_SAFE_RESET(DepthStencil);
}
