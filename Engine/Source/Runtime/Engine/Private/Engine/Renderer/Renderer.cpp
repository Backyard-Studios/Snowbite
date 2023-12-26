#include "pch.h"

#include <Engine/Renderer/Renderer.h>

#include "Engine/Graphics/D3D12/D3D12GraphicsInterface.h"

std::shared_ptr<IGraphicsInterface> FRenderer::GraphicsInterface = nullptr;

HRESULT FRenderer::BeginFrame()
{
	const HRESULT BeginResult = GraphicsInterface->BeginFrame();
	SB_CHECK_RESULT(BeginResult, "Failed to begin frame");
	return S_OK;
}

HRESULT FRenderer::EndFrame()
{
	const HRESULT EndResult = GraphicsInterface->EndFrame();
	SB_CHECK_RESULT(EndResult, "Failed to end frame");
	return S_OK;
}

HRESULT FRenderer::Initialize()
{
	GraphicsInterface = std::make_shared<FD3D12GraphicsInterface>();
	const HRESULT InitializeResult = GraphicsInterface->Initialize();
	SB_CHECK_RESULT(InitializeResult);
	return S_OK;
}

void FRenderer::Shutdown()
{
	GraphicsInterface->Destroy();
	SB_SAFE_RESET(GraphicsInterface);
}
