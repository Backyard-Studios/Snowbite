#include "pch.h"

#include <Engine/Platform/Platform.h>

#include <Dbghelp.h>

typedef enum _PROCESS_DPI_AWARENESS
{
	PROCESS_DPI_UNAWARE = 0,
	PROCESS_SYSTEM_DPI_AWARE = 1,
	PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;

typedef HRESULT (STDAPICALLTYPE *SetProcessDpiAwarenessProc)(PROCESS_DPI_AWARENESS Value);
typedef HRESULT (STDAPICALLTYPE *GetDPIForMonitorProc)(HMONITOR Monitor, UINT Type, UINT* X, UINT* Y);

struct FCrashInfo
{
	std::string MiniDumpPath;
	LPEXCEPTION_POINTERS ExceptionPointers;
	DWORD ThreadId;
};

typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
                                         PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
                                         PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
                                         PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

uint32_t FPlatform::DpiScale = 96;
WNDCLASSEX FPlatform::WindowClass = {};

HRESULT FPlatform::Initialize(const HINSTANCE Instance)
{
	SetHighDpiAwareness(true);
	DisableProcessWindowsGhosting();

	WindowClass.cbSize = sizeof(WNDCLASSEX);
	WindowClass.style = CS_DBLCLKS;
	WindowClass.lpfnWndProc = WindowProc;
	WindowClass.cbClsExtra = 0;
	WindowClass.cbWndExtra = 0;
	WindowClass.hInstance = Instance;
	WindowClass.hIcon = nullptr;
	WindowClass.hCursor = nullptr;
	WindowClass.hbrBackground = nullptr;
	WindowClass.lpszMenuName = nullptr;
	WindowClass.lpszClassName = TEXT("SnowbiteDefaultWindowClass");
	WindowClass.hIconSm = nullptr;
	if (!RegisterClassEx(&WindowClass))
		return E_FAIL;
	return S_OK;
}

void FPlatform::Shutdown()
{
	UnregisterClass(WindowClass.lpszClassName, WindowClass.hInstance);
}

void FPlatform::SetHighDpiAwareness(const bool bIsAware)
{
	const HMODULE ShCoreModule = LoadLibrary(TEXT("Shcore.dll"));
	if (!ShCoreModule)
		return;
	if (const SetProcessDpiAwarenessProc SetProcessDpiAwareness = reinterpret_cast<SetProcessDpiAwarenessProc>(
		GetProcAddress(ShCoreModule, "SetProcessDpiAwareness")))
		SetProcessDpiAwareness(bIsAware ? PROCESS_PER_MONITOR_DPI_AWARE : PROCESS_DPI_UNAWARE);
	DpiScale = CalculateDpiScale(ShCoreModule);
	FreeLibrary(ShCoreModule);
}

void FPlatform::CollectCrashInfo(const LPEXCEPTION_POINTERS ExceptionPointers,
                                 const DWORD ThreadId)
{
	FCrashInfo CrashInfo;
	CrashInfo.ExceptionPointers = ExceptionPointers;
	CrashInfo.ThreadId = ThreadId;
	CrashInfo.MiniDumpPath = "CrashDump.dmp";
	DWORD CollectThreadId = 0;
	const HANDLE CollectThread = CreateThread(nullptr, 0x8000, [](const LPVOID Param) -> DWORD
	{
		const HMODULE DebugHelperModuleHandle = LoadLibrary(TEXT("dbghelp.dll"));
		if (!DebugHelperModuleHandle)
			return GetLastError();
		const MINIDUMPWRITEDUMP WriteDumpFunction = reinterpret_cast<MINIDUMPWRITEDUMP>(GetProcAddress(
			DebugHelperModuleHandle, "MiniDumpWriteDump"));
		if (!WriteDumpFunction)
			return GetLastError();
		const FCrashInfo* CrashInfo = static_cast<const FCrashInfo*>(Param);
		const HANDLE Process = GetCurrentProcess();
		const HANDLE File = CreateFile(CrashInfo->MiniDumpPath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
		                               FILE_ATTRIBUTE_NORMAL, nullptr);
		if (File != INVALID_HANDLE_VALUE)
		{
			constexpr MINIDUMP_TYPE MiniDumpType = static_cast<MINIDUMP_TYPE>(MiniDumpWithFullMemoryInfo |
				MiniDumpFilterMemory | MiniDumpWithHandleData |
				MiniDumpWithThreadInfo | MiniDumpWithUnloadedModules);
			MINIDUMP_EXCEPTION_INFORMATION ExceptionInfo;
			ExceptionInfo.ThreadId = CrashInfo->ThreadId;
			ExceptionInfo.ExceptionPointers = CrashInfo->ExceptionPointers;
			ExceptionInfo.ClientPointers = FALSE;
			WriteDumpFunction(Process, GetCurrentProcessId(), File, MiniDumpType, &ExceptionInfo,
			                  nullptr, nullptr);
			CloseHandle(File);
		}
		if (DebugHelperModuleHandle)
			FreeLibrary(DebugHelperModuleHandle);
		return 0;
	}, &CrashInfo, 0, &CollectThreadId);
	WaitForSingleObject(CollectThread, INFINITE);
}

LONG FPlatform::SehExceptionHandler(EXCEPTION_POINTERS* ExceptionPointers)
{
	OutputDebugString(TEXT("Unhandled Exception! Creating Crash Dump..."));
	std::cout << "Unhandled Exception! Creating Crash Dump..." << std::endl;
	CollectCrashInfo(ExceptionPointers, GetCurrentThreadId());
	return EXCEPTION_CONTINUE_SEARCH;
}

uint32_t FPlatform::CalculateDpiScale(const HMODULE ShCoreModule)
{
	uint32_t X = 96;
	uint32_t Y = 96;
	if (ShCoreModule)
	{
		if (const GetDPIForMonitorProc GetDPIForMonitor = reinterpret_cast<GetDPIForMonitorProc>(GetProcAddress(
			ShCoreModule, "GetDpiForMonitor")))
		{
			constexpr POINT ZeroPoint = {0, 0};
			const HMONITOR Monitor = MonitorFromPoint(ZeroPoint, MONITOR_DEFAULTTOPRIMARY);
			UINT TempX = 0, TempY = 0;
			const HRESULT hr = GetDPIForMonitor(Monitor, 0, &TempX, &TempY);
			if (SUCCEEDED(hr) && TempX > 0 && TempY > 0)
			{
				X = TempX;
				Y = TempY;
			}
		}
		FreeLibrary(ShCoreModule);
	}
	return (X + Y) / 2;
}

LRESULT FPlatform::WindowProc(const HWND WindowHandle, const UINT Message, const WPARAM WParam, const LPARAM LParam)
{
	if (WindowHandle != nullptr && FWindowManager::IsRegistered(WindowHandle))
	{
		const std::shared_ptr<FWindow> Window = FWindowManager::GetWindow(WindowHandle);
		return Window->WndProc(Message, WParam, LParam);
	}
	return DefWindowProc(WindowHandle, Message, WParam, LParam);
}
