#include "pch.h"

#include <Engine/Graphics/Renderer.h>

#include "Engine/Graphics/D3D12/D3D12RHI.h"
#include "Engine/Graphics/D3D12/D3D12Utils.h"

FRenderer::FRenderer(const FRendererSettings& InSettings)
	: Settings(InSettings)
{
	FD3D12RHISettings RHISettings;
	RHISettings.BufferingMode = Settings.BufferingMode;
	RHISettings.Window = Settings.Window;
	RHISettings.bEnableDebugLayer = true;
	RHISettings.bEnableGPUValidation = true;
	RHI = std::make_shared<FD3D12RHI>(RHISettings);
	const HRESULT InitializeResult = RHI->Initialize();
	SB_ASSERT_CRITICAL(SUCCEEDED(InitializeResult), E_RHI_INITIALIZATION_FAILED, "Failed to initialize renderer");
	RHI->SetBackBufferClearColor(ClearColor);
	SB_LOG_INFO("Successfully initialized renderer with {}", RHI->GetName());
	SB_LOG_DEBUG("Using {} mode", GetBufferingModeName(Settings.BufferingMode));
}

FRenderer::~FRenderer()
{
	RHI->Shutdown();
	SB_SAFE_RESET(RHI);
}

void FRenderer::Resize(const uint32_t InWidth, const uint32_t InHeight) const
{
}

void FRenderer::SetClearColor(const FClearColor InClearColor)
{
	ClearColor = InClearColor;
	RHI->SetBackBufferClearColor(ClearColor);
}

HRESULT FRenderer::BeginFrame() const
{
	const HRESULT PrepareResult = RHI->PrepareNextFrame();
	SB_D3D_FAILED_RETURN(PrepareResult);
	return S_OK;
}

HRESULT FRenderer::EndFrame() const
{
	const HRESULT PresentResult = RHI->PresentFrame();
	SB_D3D_FAILED_RETURN(PresentResult);
	return S_OK;
}
