#include "pch.h"

#include "Engine/Platform/Platform.h"

#if SB_EXECUTABLE_CONSOLE
int main(int ArgumentCount, char** ArgumentArray)
#else
int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, PSTR CmdLine, int CmdShow)
#endif
{
	if (FAILED(FPlatform::Initialize()))
		return GetLastError();
	__try
	{
	}
	__except (FPlatform::SehExceptionHandler(GetExceptionInformation()))
	{
		return -1;
	}
	FPlatform::Shutdown();
	return 0;
}
