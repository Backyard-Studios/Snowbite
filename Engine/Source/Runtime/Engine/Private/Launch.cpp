#include "pch.h"

#include "Engine/Engine.h"
#include "Engine/Platform/Platform.h"

#if SB_EXECUTABLE_CONSOLE
#define SB_HINSTANCE GetModuleHandle(nullptr)

int main(const int ArgumentCount, char** ArgumentArray)
#else
#define SB_HINSTANCE Instance

int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, PSTR CmdLine, int CmdShow)
#endif
{
	if (FAILED(FPlatform::Initialize(SB_HINSTANCE)))
		return GetLastError();
	__try
	{
		FEngine::EntryPoint(ArgumentCount, ArgumentArray);
	}
	__except (FPlatform::SehExceptionHandler(GetExceptionInformation()))
	{
		return EXIT_FAILURE;
	}
	FPlatform::Shutdown();
	return 0;
}
