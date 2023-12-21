#include "pch.h"

#include <Engine/Engine.h>

#include "imgui.h"
#include "Engine/Graphics/D3D12/D3D12Utils.h"

FEngine::FEngine(const FArgumentParser& InArgumentParser, std::shared_ptr<IApplication> InApplication)
	: ArgumentParser(InArgumentParser), Application(std::move(InApplication))
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
	LayerStack = std::make_unique<FLayerStack>();
	const HRESULT ApplicationInitializeResult = Application->Initialize();
	SB_D3D_ASSERT_RETURN(ApplicationInitializeResult, "Failed to initialize application");
	SB_LOG_INFO("Application '{}' initialized", Application->GetName());
	if (!IsHeadless())
	{
		FWindowDesc MainWindowDesc{};
		MainWindow = std::make_shared<FWindow>(MainWindowDesc);

		FRendererSettings RendererSettings;
		RendererSettings.BufferingMode = EBufferingMode::DoubleBuffering;
		RendererSettings.Window = MainWindow;
		Renderer = std::make_shared<FRenderer>(RendererSettings);
		Renderer->SetClearColor(FClearColor(0.083f, 0.083f, 0.083f, 1.0f));
		MainWindow->SetOnClientResizeCallback([this](const uint32_t InWidth, const uint32_t InHeight)
		{
			Renderer->Resize(InWidth, InHeight);
		});
	}
	return S_OK;
}

void FEngine::Run()
{
	// [UpdateLoop]
	// |-> [WindowHandling]
	// |-> [Input]
	// |-> [LogicUpdate]
	// |-> [RenderingBeginFrame]
	// |	|-> [RenderingUpdate]
	// |	|-> [UIUpdate]
	// |-> [RenderingEndFrame]
	// |-> [DeferredChangesHandling]
	while (!bIsShutdownRequested)
	{
		if (!IsHeadless())
		{
			MSG Message;
			while (PeekMessage(&Message, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&Message);
				DispatchMessage(&Message);
			}
			if (MainWindow->IsClosed())
				RequestShutdown();

			if (GetAsyncKeyState(VK_F11) & 1)
				MainWindow->SetFullscreen(!MainWindow->IsFullscreen());

			for (const std::shared_ptr<ILayer>& Layer : LayerStack->GetLayers())
				Layer->OnLogicUpdate();
			for (const std::shared_ptr<ILayer>& Layer : Application->GetLayerStack()->GetLayers())
				Layer->OnLogicUpdate();

			const HRESULT BeginFrameResult = Renderer->BeginFrame();
			if (FAILED(BeginFrameResult))
				break;
			{
				for (const std::shared_ptr<ILayer>& Layer : LayerStack->GetLayers())
					Layer->OnRenderUpdate();
				for (const std::shared_ptr<ILayer>& Layer : Application->GetLayerStack()->GetLayers())
					Layer->OnRenderUpdate();

				Renderer->BeginUIFrame();
				{
					for (const std::shared_ptr<ILayer>& Layer : LayerStack->GetLayers())
						Layer->OnUIUpdate();
					for (const std::shared_ptr<ILayer>& Layer : Application->GetLayerStack()->GetLayers())
						Layer->OnUIUpdate();
				}
				Renderer->EndUIFrame();
			}
			const HRESULT EndFrameResult = Renderer->EndFrame();
			if (FAILED(EndFrameResult))
				break;
			LayerStack->HandleDeferredLayerChanges();
			Application->GetLayerStack()->HandleDeferredLayerChanges();
		}
	}
}

HRESULT FEngine::Shutdown(const HRESULT ExitCode)
{
	SB_LOG_INFO("Shutting down...");
	LayerStack->Shutdown();
	if (!IsHeadless())
	{
		MainWindow->SetOnClientResizeCallback(nullptr);
		SB_SAFE_RESET(Renderer)
		SB_SAFE_RESET(MainWindow)
	}
	Application->Shutdown();
	SB_LOG_INFO("Shutdown complete");
	SB_SAFE_RESET(LayerStack)
	return ExitCode;
}

std::shared_ptr<FEngine> GetEngine()
{
	return FEngine::EngineInstance;
}
