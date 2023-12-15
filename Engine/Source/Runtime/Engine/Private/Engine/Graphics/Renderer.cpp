#include "pch.h"

#include <Engine/Graphics/Renderer.h>

FRenderer::FRenderer(const FRendererSettings& InSettings)
	: Settings(InSettings)
{
	FGraphicsDeviceSettings GraphicsDeviceSettings;
#ifdef SB_DEBUG
	GraphicsDeviceSettings.bEnableDebugLayer = true;
	GraphicsDeviceSettings.bEnableGPUValidation = true;
#endif
	GraphicsDeviceSettings.Window = Settings.Window;
	GraphicsDeviceSettings.BufferingMode = Settings.BufferingMode;
	GraphicsDevice = std::make_shared<FGraphicsDevice>(GraphicsDeviceSettings);
	SB_LOG_INFO("D3D12 renderer initialized");
}

FRenderer::~FRenderer()
{
	SB_SAFE_RESET(GraphicsDevice);
}

void FRenderer::Resize(const uint32_t InWidth, const uint32_t InHeight) const
{
	GraphicsDevice->Resize(InWidth, InHeight);
}

void FRenderer::SetClearColor(const FClearColor InClearColor)
{
	ClearColor = InClearColor;
}

void FRenderer::BeginFrame() const
{
	GraphicsDevice->BeginFrame(ClearColor);
}

void FRenderer::EndFrame() const
{
	GraphicsDevice->EndFrame();
}
