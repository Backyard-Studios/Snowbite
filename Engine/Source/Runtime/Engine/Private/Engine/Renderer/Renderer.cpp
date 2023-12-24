#include "pch.h"

#include <Engine/Renderer/Renderer.h>

std::shared_ptr<IGraphicsInterface> FRenderer::GraphicsInterface = nullptr;

HRESULT FRenderer::Initialize()
{
	SB_CHECK_RESULT(GraphicsInterface->Initialize(), "Failed to initialize graphics interface");
	return S_OK;
}

void FRenderer::Shutdown()
{
	GraphicsInterface->Destroy();
	SB_SAFE_RESET(GraphicsInterface);
}
