#pragma once

#include <Engine/Core/Definitions.h>
#include <cstdint>
#include <memory>
#include <imgui.h>
// ReSharper disable once CppUnusedIncludeDirective
#include <Windows.h>

#include "Engine/Application/Application.h"

/**
 * Instantiates, runs and destroys the engine
 */
SNOWBITE_API uint32_t LaunchSnowbite(int ArgumentCount, char* Arguments[], ImGuiContext* GlobalImGuiContext,
                                     std::shared_ptr<IApplication> Application);

#if defined(SB_DEBUG) || defined(SB_RELEASE)
#	define SB_MAIN_SIGNATURE int main(int ArgumentCount, char* Arguments[])
#	define SB_MAIN_ARGUMENTS ArgumentCount, Arguments
#else
#	include <stdlib.h>
#	define SB_MAIN_SIGNATURE int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#	define SB_MAIN_ARGUMENTS __argc, __argv
#endif

#define LAUNCH_SNOWBITE(ApplicationClass) \
	static ImGuiContext* GlobalImGuiContext = nullptr; \
	SB_MAIN_SIGNATURE \
	{ \
		GlobalImGuiContext = ImGui::CreateContext(); \
		return LaunchSnowbite(SB_MAIN_ARGUMENTS, GlobalImGuiContext, std::make_shared<ApplicationClass>()); \
	}
