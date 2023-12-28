#include "pch.h"

#include <Engine/Platform/Launch.h>

uint32_t GuardedMain(const int ArgumentCount, char** Arguments)
{
	const HRESULT Result = FEngine::EntryPoint(ArgumentCount, Arguments);
	if (FAILED(Result))
	{
		FPlatform::PrintHRESULT(Result);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

uint32_t LaunchSnowbite(const int ArgumentCount, char** Arguments)
{
	uint32_t ExitCode = EXIT_SUCCESS;
	__try
	{
		ExitCode = GuardedMain(ArgumentCount, Arguments);
	}
	__except (FPlatform::SehExceptionHandler(GetExceptionInformation()))
	{
		(void)0;
	}
	return ExitCode;
}
