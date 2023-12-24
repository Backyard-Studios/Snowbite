#include "pch.h"

#include "Engine/Platform/Platform.h"
#include "Engine/Engine.h"

void GuardedMain(const int ArgumentCount, char** ArgumentArray)
{
	const HRESULT Result = FEngine::EntryPoint(ArgumentCount, ArgumentArray);
	if (FAILED(Result))
		FPlatform::Fatal("Engine entry point failed", Result);
}

#if SB_EXECUTABLE_CONSOLE
#define SB_ARGUMENTS ArgumentCount, ArgumentArray
#define SB_HINSTANCE GetModuleHandle(nullptr)

int main(const int ArgumentCount, char** ArgumentArray)
#else
#define SB_ARGUMENTS __argc, __argv
#define SB_HINSTANCE Instance

int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, PSTR CmdLine, int CmdShow)
#endif
{
	if (FAILED(FPlatform::Initialize(SB_HINSTANCE)))
		return GetLastError();
	__try
	{
		GuardedMain(SB_ARGUMENTS);
	}
	__except (FPlatform::SehExceptionHandler(GetExceptionInformation()))
	{
		return EXIT_FAILURE;
	}
	FPlatform::Shutdown();
	return 0;
}
