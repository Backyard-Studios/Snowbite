#pragma once

#include <Engine/Core/Definitions.h>

// ReSharper disable once CppUnusedIncludeDirective
#include <Engine/Core/Logging.h>
#include <cstdint>

/**
 * Launches the engine in a exception safe manner and returns the process return code.
 *
 * @note Should only be called once from the application through the `LAUNCH_SNOWBITE()` macro.
 *
 * @param	ArgumentCount	Number of command line arguments
 * @param	Arguments		Array of command line arguments
 *
 * @return	Process return code.
 */
[[nodiscard]] SNOWBITE_API uint32_t LaunchSnowbite(int ArgumentCount, char** Arguments);

#ifdef SB_EXECUTABLE_CONSOLE
#	define SB_ENTRY_POINT_ARGUMENTS ArgumentCount, ArgumentArray
#	define SB_ENTRY_POINT_HEADER \
		int main(const int ArgumentCount, char** ArgumentArray)
#elif defined(SB_EXECUTABLE_WINDOWED)
#	include <Windows.h>
#	define SB_ENTRY_POINT_ARGUMENTS __argc, __argv
#	define SB_ENTRY_POINT_HEADER \
		int WINAPI WinMain(const HINSTANCE Instance, HINSTANCE PrevInstance, PSTR CmdLine, int CmdShow)
#endif

/**
 * Macro to be used by the application to launch the engine.
 *
 * @note Should only be called once from the application.
 */
#define LAUNCH_SNOWBITE() \
	SB_ENTRY_POINT_HEADER \
	{ \
		FLogging::Initialize(); \
		uint32_t ExitCode = LaunchSnowbite(SB_ENTRY_POINT_ARGUMENTS); \
		FLogging::Shutdown(); \
		return ExitCode; \
	}
