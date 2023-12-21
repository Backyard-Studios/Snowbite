#include <Engine/Platform/Launch.h>

#include "imgui.h"
#include "Engine/Engine.h"
#include "Engine/Core/Logging.h"
#include "Engine/Graphics/D3D12/D3D12RHI.h"

class FTestLayer : public ILayer
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
		GetEngine()->GetApplication()->GetLayerStack()->ShowDebugUI("Application Layer Stack");
		ImGui::SetNextWindowSize(ImVec2(180, 80));
		ImGui::Begin(GetName(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		{
			ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);
			bool bIsVSyncEnabled = GetEngine()->GetRenderer()->IsVSyncEnabled();
			ImGui::Checkbox("VSync", &bIsVSyncEnabled);
			GetEngine()->GetRenderer()->SetVSync(bIsVSyncEnabled);
		}
		ImGui::End();
	}

	ELayerType GetType() const override { return ELayerType::Overlay; }
	const char* GetName() const override { return "TestLayer"; }

private:
};

class FSandboxApplication : public IApplication
{
public:
	HRESULT Initialize() override
	{
		IApplication::Initialize();
		TestLayer = std::make_shared<FTestLayer>();
		LayerStack->PushLayer(TestLayer);
		SB_LOG_INFO("Sandbox application initialized");
		return S_OK;
	}

	void Shutdown() override
	{
		IApplication::Shutdown();
	}

	const char* GetName() const override { return "Sandbox"; }

private:
	std::shared_ptr<FTestLayer> TestLayer;
};

LAUNCH_SNOWBITE(FSandboxApplication)
