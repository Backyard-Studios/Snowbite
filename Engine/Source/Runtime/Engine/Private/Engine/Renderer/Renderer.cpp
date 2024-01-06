#include "pch.h"

#include <Engine/Renderer/Renderer.h>

std::shared_ptr<FGraphicsDevice> FRenderer::GraphicsDevice;

HRESULT FRenderer::BeginFrame()
{
	SB_CHECK(GraphicsDevice->BeginFrame());
	return S_OK;
}

HRESULT FRenderer::EndFrame()
{
	SB_CHECK(GraphicsDevice->EndFrame());
	return S_OK;
}

HRESULT FRenderer::Resize(const uint32_t NewWidth, const uint32_t NewHeight)
{
	SB_CHECK(GraphicsDevice->Resize(NewWidth, NewHeight));
	return S_OK;
}

HRESULT FRenderer::Initialize(const std::shared_ptr<FWindow>& Window)
{
	GraphicsDevice = std::make_shared<FGraphicsDevice>();
	constexpr FGraphicsDeviceDesc GraphicsDeviceDesc;
	SB_CHECK(GraphicsDevice->Initialize(Window, GraphicsDeviceDesc));
	return S_OK;
}

void FRenderer::Shutdown()
{
	SB_SAFE_DESTROY(GraphicsDevice);
}
