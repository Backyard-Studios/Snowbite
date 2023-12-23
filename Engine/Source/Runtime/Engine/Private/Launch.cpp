#include "pch.h"

#include <ShellScalingAPI.h>
#pragma comment(lib, "shcore.lib")

#if SB_EXECUTABLE_CONSOLE
int main(int ArgumentCount, char** ArgumentArray)
#else
int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, PSTR CmdLine, int CmdShow)
#endif
{
	const HRESULT SetDpiAwarenessResult = SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
	if (FAILED(SetDpiAwarenessResult))
		return SetDpiAwarenessResult;
	return 0;
}
