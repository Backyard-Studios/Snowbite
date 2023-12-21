#include "pch.h"

#include <Engine/Application/LayerStack.h>

#include "imgui.h"

FLayerStack::FLayerStack() = default;
FLayerStack::~FLayerStack() = default;

void FLayerStack::Shutdown()
{
	for (std::shared_ptr<ILayer>& Layer : Layers)
		PopLayer(Layer);
	HandleDeferredLayerChanges();
	Layers.clear();
}

void FLayerStack::PushLayer(const std::shared_ptr<ILayer>& Layer)
{
	LayersToPush.emplace(Layer);
}

void FLayerStack::PopLayer(const std::shared_ptr<ILayer>& Layer)
{
	LayersToPop.emplace(Layer);
}

void FLayerStack::HandleDeferredLayerChanges()
{
	while (!LayersToPush.empty())
	{
		std::shared_ptr<ILayer> Layer = LayersToPush.front();
		Layers.push_back(Layer);
		Layer->OnAttach();
		LayersToPush.pop();
	}
	while (!LayersToPop.empty())
	{
		std::shared_ptr<ILayer> Layer = LayersToPop.front();
		auto Iterator = std::ranges::find(Layers, Layer);
		if (Iterator != Layers.end())
		{
			Layer->OnDetach();
			Layers.erase(Iterator);
		}
		LayersToPop.pop();
	}
	std::ranges::sort(Layers, [](const std::shared_ptr<ILayer>& A, const std::shared_ptr<ILayer>& B)
	{
		return static_cast<int>(A->GetType()) < static_cast<int>(B->GetType());
	});
}

void FLayerStack::ShowDebugUI(const char* Title)
{
	ImGui::SetNextWindowSizeConstraints(ImVec2(180, 100), ImVec2(180, FLT_MAX));
	ImGui::Begin(Title, nullptr, ImGuiWindowFlags_NoCollapse);
	if (!Layers.empty())
	{
		ImGui::Columns(2);
		for (const std::shared_ptr<ILayer>& Layer : Layers)
		{
			ImGui::Text("%s", Layer->GetName());
			ImGui::NextColumn();
			if (ImGui::Button("Pop"))
				PopLayer(Layer);
		}
		ImGui::Columns(1);
	}
	else
	{
		ImGui::Text("No layers");
	}
	ImGui::End();
}
