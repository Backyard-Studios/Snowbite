#include "pch.h"

#include <Engine/Engine.h>

FEngine::FEngine(const FArgumentParser& InArgumentParser)
	: ArgumentParser(InArgumentParser)
{
}

void FEngine::RequestShutdown()
{
	bIsShutdownRequested = true;
}

HRESULT FEngine::Initialize()
{
	if (ArgumentParser.HasArgument("headless"))
		bIsHeadless = true;
	SB_LOG_INFO("Snowbite Engine");
	SB_LOG_INFO("\t- Version: v{}.{}.{}-{}", SNOWBITE_VERSION_MAJOR, SNOWBITE_VERSION_MINOR, SNOWBITE_VERSION_PATCH,
	            SNOWBITE_VERSION_BRANCH);
	SB_LOG_INFO("\t- Mode: {}", IsHeadless() ? "Headless" : "Standard");
	SB_LOG_INFO("Main thread id is {}", GetCurrentThreadId());
	if (!IsHeadless())
	{
		FWindowDesc MainWindowDesc{};
		MainWindow = std::make_shared<FWindow>(MainWindowDesc);

		FRendererSettings RendererSettings;
		RendererSettings.BufferingMode = EBufferingMode::TripleBuffering;
		RendererSettings.Window = MainWindow;
		Renderer = std::make_shared<FRenderer>(RendererSettings);
		Renderer->SetClearColor(FClearColor(1.0f, 0.0f, 0.396f, 1.0f));
		MainWindow->SetOnClientResizeCallback([this](const uint32_t InWidth, const uint32_t InHeight)
		{
			Renderer->Resize(InWidth, InHeight);
		});
	}
	return S_OK;
}

void FEngine::Run()
{
	while (!bIsShutdownRequested)
	{
		if (!IsHeadless())
		{
			MSG Message;
			if (PeekMessage(&Message, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&Message);
				DispatchMessage(&Message);
			}
			if (MainWindow->IsClosed())
				RequestShutdown();

			Renderer->BeginFrame();
			{
			}
			Renderer->EndFrame();
		}
	}
}

HRESULT FEngine::Shutdown(const HRESULT ExitCode)
{
	SB_LOG_INFO("Shutting down...");
	if (!IsHeadless())
	{
		MainWindow->SetOnClientResizeCallback(nullptr);
		SB_SAFE_RESET(Renderer)
		SB_SAFE_RESET(MainWindow)
	}
	return ExitCode;
}

std::shared_ptr<FEngine> GetEngine()
{
	return FEngine::EngineInstance;
}
