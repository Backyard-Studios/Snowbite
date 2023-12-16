#pragma once

#include <Engine/Core/Definitions.h>
#include <cstdint>

/**
 * Instantiates, runs and destroys the engine
 */
SNOWBITE_API uint32_t LaunchSnowbite(int ArgumentCount, char* Arguments[]);

#if defined(SB_DEBUG) || defined(SB_RELEASE)
#	define SB_MAIN_SIGNATURE int main(int ArgumentCount, char* Arguments[])
#	define SB_MAIN_ARGUMENTS ArgumentCount, Arguments
#else
#	include <stdlib.h>
#	include <Windows.h>
#	define SB_MAIN_SIGNATURE int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#	define SB_MAIN_ARGUMENTS __argc, __argv
#endif

#define LAUNCH_SNOWBITE() \
	SB_MAIN_SIGNATURE \
	{ \
		return LaunchSnowbite(SB_MAIN_ARGUMENTS); \
	}
