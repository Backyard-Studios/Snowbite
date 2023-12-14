#include "pch.h"

#include <Engine/Platform/Launch.h>

#include "Engine/Engine.h"

#include <windows.h>
#include <Dbghelp.h>

typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
                                         PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
                                         PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
                                         PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

LONG WINAPI HandleException(_EXCEPTION_POINTERS* apExceptionInfo)
{
	const HMODULE DebugHelperModuleHandle = LoadLibrary(TEXT("dbghelp.dll"));
	const MINIDUMPWRITEDUMP WriteDumpFunction = reinterpret_cast<MINIDUMPWRITEDUMP>(GetProcAddress(
		DebugHelperModuleHandle, "MiniDumpWriteDump"));
	const HANDLE FileHandle = CreateFile(TEXT("core.dmp"), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS,
	                                     FILE_ATTRIBUTE_NORMAL, nullptr);
	_MINIDUMP_EXCEPTION_INFORMATION ExInfo;
	ExInfo.ThreadId = GetCurrentThreadId();
	ExInfo.ExceptionPointers = apExceptionInfo;
	ExInfo.ClientPointers = FALSE;
	WriteDumpFunction(GetCurrentProcess(), GetCurrentProcessId(), FileHandle, MiniDumpNormal, &ExInfo, nullptr,
	                  nullptr);
	CloseHandle(FileHandle);
	return EXCEPTION_CONTINUE_SEARCH;
}

uint32_t LaunchSnowbite(int argc, char* argv[])
{
	SetUnhandledExceptionFilter(HandleException);
	FEngine::EngineInstance = std::make_shared<FEngine>();
	const HRESULT initializeResult = GetEngine()->Initialize();
	if (SUCCEEDED(initializeResult))
		GetEngine()->Run();
	const HRESULT shutdownResult = GetEngine()->Shutdown(initializeResult);
	if (GetEngine().use_count() > 2)
		return 1;
	SB_SAFE_RESET(FEngine::EngineInstance);
	return shutdownResult;
}
