﻿#include "pch.h"

#include <Engine/Engine.h>

#include "Engine/Renderer/GraphicsDevice.h"
#include "Engine/Renderer/Renderer.h"

/**
 * Calls the specified lifecycle method. Only use this macro if the lifecycle method returns a result.
 * @param Method The lifecycle method to call.
 * @param ... The arguments to pass to the lifecycle method.
 */
#define SB_LIFECYCLE_WITH_RESULT(Method, ...) \
	const HRESULT SB_UNIQUE_NAME(Result) = Method(__VA_ARGS__); \
	if (FAILED(SB_UNIQUE_NAME(Result))) \
	{ \
		return SB_UNIQUE_NAME(Result); \
	}

/**
 * Only exists to distinguish between normal methods and lifecycle methods.
 * Calls the specified lifecycle method. Only use this macro if the lifecycle method does *not* return a result.
 * @param Method The lifecycle method to call.
 * @param ... The arguments to pass to the lifecycle method.
 */
#define SB_LIFECYCLE(Method, ...) \
	Method(__VA_ARGS__);

bool FEngine::bShouldExit = false;
uint32_t FEngine::ExitCode = EXIT_SUCCESS;
std::shared_ptr<FWindow> FEngine::MainWindow;

HRESULT FEngine::EntryPoint(int ArgumentCount, char* ArgumentArray[])
{
	SB_LOG_INFO("Snowbite v{}.{}.{}-{}", SNOWBITE_VERSION_MAJOR, SNOWBITE_VERSION_MINOR, SNOWBITE_VERSION_PATCH,
	            SNOWBITE_VERSION_BRANCH);
	SB_LIFECYCLE_WITH_RESULT(PreInitialize)
	SB_LIFECYCLE_WITH_RESULT(Initialize)
	SB_LIFECYCLE_WITH_RESULT(PostInitialize)
	while (!bShouldExit)
	{
		SB_LIFECYCLE_WITH_RESULT(EarlyUpdate)
		SB_LIFECYCLE(Update)
		SB_LIFECYCLE_WITH_RESULT(LateUpdate)
	}
	SB_LIFECYCLE(BeforeShutdown)
	SB_LIFECYCLE(Shutdown)
	SB_LIFECYCLE(AfterShutdown)
	return ExitCode;
}

void FEngine::RequestExit(const uint32_t InExitCode)
{
	ExitCode = InExitCode;
	bShouldExit = true;
}

void FEngine::Exit(const uint32_t InExitCode)
{
	ExitCode = InExitCode;
	FPlatform::Fatal();
}

HRESULT FEngine::PreInitialize()
{
	SB_CHECK(FPlatform::Initialize(GetModuleHandle(nullptr)));
	return S_OK;
}

HRESULT FEngine::Initialize()
{
	FWindowDesc WindowDesc;
	WindowDesc.bShouldRequestExitOnClose = true;
	WindowDesc.bShouldAutoShow = false;
	MainWindow = std::make_shared<FWindow>(WindowDesc);
	FWindowManager::Register(MainWindow);

	SB_CHECK(FRenderer::Initialize(MainWindow));
	return S_OK;
}

HRESULT FEngine::PostInitialize()
{
	MainWindow->Show();
	return S_OK;
}

HRESULT FEngine::EarlyUpdate()
{
	// Handle messages and input
	FWindowManager::HandleWindowMessages();
	if (MainWindow->WasResized())
	{
		SB_CHECK(FRenderer::Resize(MainWindow->GetSize().X, MainWindow->GetSize().Y));
		MainWindow->ClearResizeFlag();
	}
	return S_OK;
}

void FEngine::Update()
{
	// Update game logic and physics
}

HRESULT FEngine::LateUpdate()
{
	SB_CHECK(FRenderer::BeginFrame());
	{
		// Draw scene
	}
	SB_CHECK(FRenderer::EndFrame())
	return S_OK;
}

void FEngine::BeforeShutdown()
{
}

void FEngine::Shutdown()
{
	FRenderer::Shutdown();
	FWindowManager::Unregister(MainWindow);
	MainWindow.reset();
	MainWindow = nullptr;
}

void FEngine::AfterShutdown()
{
	FPlatform::Shutdown();
}
