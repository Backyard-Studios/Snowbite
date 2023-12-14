#pragma once

#include <Engine/Core/Definitions.h>
#include <cstdint>

/**
 * Instantiates, runs and destroys the engine
 */
SNOWBITE_API uint32_t LaunchSnowbite(int argc, char* argv[]);

#ifdef SB_DEBUG
#	define SB_MAIN_SIGNATURE int main(int argc, char* argv[])
#else
#	include <Windows.h>
#	define SB_MAIN_SIGNATURE int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#endif

#define LAUNCH_SNOWBITE() \
	SB_MAIN_SIGNATURE \
	{ \
		return LaunchSnowbite(argc, argv); \
	}
