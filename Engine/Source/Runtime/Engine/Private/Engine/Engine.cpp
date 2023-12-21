#include "pch.h"

#include <Engine/Engine.h>

#include "imgui.h"

class TestLayer : public ILayer
{
public:
	void OnAttach() override
	{
	}

	void OnDetach() override
	{
	}

	void OnLogicUpdate() override
	{
	}

	void OnRenderUpdate() override
	{
	}

	void OnUIUpdate() override
	{
		ImGui::Begin(GetName());
		ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);
		bool bIsVSyncEnabled = GetEngine()->GetRenderer()->IsVSyncEnabled();
		ImGui::Checkbox("VSync", &bIsVSyncEnabled);
		GetEngine()->GetRenderer()->SetVSync(bIsVSyncEnabled);
		ImGui::End();
	}

	ELayerType GetType() const override { return ELayerType::Overlay; }
	const char* GetName() const override { return "TestLayer"; }

private:
};

std::shared_ptr<TestLayer> TestLayerInstance = std::make_shared<TestLayer>();

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
	LayerStack = std::make_unique<FLayerStack>();
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
		LayerStack->PushLayer(TestLayerInstance);
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

			if (GetAsyncKeyState(VK_INSERT) & 1)
				LayerStack->PushLayer(TestLayerInstance);
			if (GetAsyncKeyState(VK_END) & 1)
				LayerStack->PopLayer(TestLayerInstance);

			for (const std::shared_ptr<ILayer>& Layer : LayerStack->GetLayers())
				Layer->OnLogicUpdate();

			const HRESULT BeginFrameResult = Renderer->BeginFrame();
			if (FAILED(BeginFrameResult))
				break;
			{
				for (const std::shared_ptr<ILayer>& Layer : LayerStack->GetLayers())
					Layer->OnRenderUpdate();

				Renderer->BeginUIFrame();
				for (const std::shared_ptr<ILayer>& Layer : LayerStack->GetLayers())
					Layer->OnUIUpdate();
				LayerStack->ShowDebugUI();
				Renderer->EndUIFrame();
			}
			const HRESULT EndFrameResult = Renderer->EndFrame();
			if (FAILED(EndFrameResult))
				break;
			LayerStack->HandleDeferredLayerChanges();
		}
	}
}

HRESULT FEngine::Shutdown(const HRESULT ExitCode)
{
	SB_LOG_INFO("Shutting down...");
	LayerStack->Shutdown();
	TestLayerInstance.reset();
	if (!IsHeadless())
	{
		MainWindow->SetOnClientResizeCallback(nullptr);
		SB_SAFE_RESET(Renderer)
		SB_SAFE_RESET(MainWindow)
	}
	SB_SAFE_RESET(LayerStack)
	return ExitCode;
}

std::shared_ptr<FEngine> GetEngine()
{
	return FEngine::EngineInstance;
}
