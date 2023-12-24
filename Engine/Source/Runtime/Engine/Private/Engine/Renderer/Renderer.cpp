#include "pch.h"

#include <Engine/Renderer/Renderer.h>

#include "Engine/Graphics/D3D12/D3D12GraphicsInterface.h"

std::shared_ptr<IGraphicsInterface> FRenderer::GraphicsInterface = nullptr;

HRESULT FRenderer::Initialize()
{
	GraphicsInterface = std::make_shared<FD3D12GraphicsInterface>();
	SB_CHECK_RESULT(GraphicsInterface->Initialize(), "Failed to initialize graphics interface");
	return S_OK;
}

void FRenderer::Shutdown()
{
	GraphicsInterface->Destroy();
	SB_SAFE_RESET(GraphicsInterface);
}
