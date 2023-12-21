#pragma once

#include <Engine/Core/Definitions.h>

#include "D3D12CommandList.h"
#include "D3D12DepthStencil.h"
#include "D3D12Device.h"
#include "D3D12SwapChain.h"
#include "Engine/Graphics/BufferingMode.h"
#include "Engine/Graphics/ClearColor.h"
#include "Engine/Graphics/RHI.h"
#include "Engine/Graphics/Window.h"

struct SNOWBITE_API FD3D12RHISettings
{
	EBufferingMode BufferingMode = EBufferingMode::TripleBuffering;
	std::shared_ptr<FWindow> Window;
	bool bEnableDebugLayer = false;
	bool bEnableGPUValidation = false;
};

struct FFrameContext
{
	std::shared_ptr<FD3D12Fence> Fence;
	std::shared_ptr<FD3D12CommandList> CommandList;
};

class SNOWBITE_API FD3D12RHI : public IStaticRHI
{
public:
	FD3D12RHI(const FD3D12RHISettings& InSettings);
	~FD3D12RHI() override;

	HRESULT Initialize() override;
	void Shutdown() override;

	HRESULT PrepareNextFrame() override;
	HRESULT PresentFrame(bool bShouldVSync) override;
	HRESULT WaitForFrame(uint32_t Index) override;
	HRESULT FlushFrames(uint32_t Count) override;

	void BeginUIFrame() override;
	void EndUIFrame() override;

	HRESULT Resize(uint32_t InWidth, uint32_t InHeight) override;

	void SetBackBufferClearColor(const FClearColor& ClearColor) override;

	uint32_t GetBufferCount() const override;
	uint32_t GetBufferIndex() const override;

	const char* GetName() const override { return "D3D12"; }
	ERHIType GetType() const override { return ERHIType::D3D12; }

private:
	HRESULT CreateSwapChain();

	HRESULT InitializeFrameContexts();
	void DestroyFrameContexts();

	HRESULT CreateDepthStencil();
	void DestroyDepthStencil();

private:
	FD3D12RHISettings Settings;

	uint32_t BufferCount = 0;
	uint32_t BufferIndex = 0;

	uint32_t Width = 0;
	uint32_t Height = 0;

#pragma region DebugLayer
	ComPointer<IDXGIDebug1> DXGIDebug;
#pragma endregion

	ComPointer<IDXGIFactory7> Factory;
	std::shared_ptr<FD3D12Device> Device;
	ComPointer<ID3D12CommandQueue> GraphicsCommandQueue;
	std::shared_ptr<FD3D12SwapChain> SwapChain;

	std::vector<FFrameContext> FrameContexts;
	FClearColor BackBufferClearColor;

	ComPointer<ID3D12DescriptorHeap> DSVDescriptorHeap;
	ComPointer<ID3D12DescriptorHeap> SRVDescriptorHeap;

	std::shared_ptr<FD3D12DepthStencil> DepthStencil;
};
