#pragma once

#include <Engine/Core/Definitions.h>
#include <cstdint>

/**
 * Instantiates, runs and destroys the engine
 */
SNOWBITE_API uint32_t LaunchSnowbite(int ArgumentCount, char* Arguments[]);

#ifdef SB_DEBUG
#	define SB_MAIN_SIGNATURE int main(int ArgumentCount, char* Arguments[])
#else
#	include <Windows.h>
#	define SB_MAIN_SIGNATURE int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#endif

#define LAUNCH_SNOWBITE() \
	SB_MAIN_SIGNATURE \
	{ \
		return LaunchSnowbite(ArgumentCount, Arguments); \
	}
