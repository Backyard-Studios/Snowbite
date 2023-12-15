#include "pch.h"

#include <Engine/Platform/Launch.h>

#include "Engine/Engine.h"

#include <windows.h>
#include <Dbghelp.h>
#include <format>

typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
                                         PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
                                         PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
                                         PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

// TODO: Investigate if this also retrieves the callstack and memory from the engine.dll
LONG WINAPI HandleException(_EXCEPTION_POINTERS* ExceptionInfo)
{
	SB_LOG_WARN("Creating crash dump...");
	const HMODULE DebugHelperModuleHandle = LoadLibrary(TEXT("dbghelp.dll"));
	const MINIDUMPWRITEDUMP WriteDumpFunction = reinterpret_cast<MINIDUMPWRITEDUMP>(GetProcAddress(
		DebugHelperModuleHandle, "MiniDumpWriteDump"));
	const HANDLE FileHandle = CreateFile(TEXT("core.dmp"), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS,
	                                     FILE_ATTRIBUTE_NORMAL, nullptr);
	constexpr MINIDUMP_TYPE MinidumpType = static_cast<MINIDUMP_TYPE>(
		MiniDumpNormal |
		MiniDumpWithDataSegs |
		MiniDumpWithHandleData |
		MiniDumpWithFullMemoryInfo |
		MiniDumpWithThreadInfo |
		MiniDumpWithUnloadedModules
	);
	_MINIDUMP_EXCEPTION_INFORMATION ExInfo;
	ExInfo.ThreadId = GetCurrentThreadId();
	ExInfo.ExceptionPointers = ExceptionInfo;
	ExInfo.ClientPointers = FALSE;
	const BOOL WriteDumpResult = WriteDumpFunction(GetCurrentProcess(), GetCurrentProcessId(), FileHandle, MinidumpType,
	                                               &ExInfo, nullptr,
	                                               nullptr);
	if (!WriteDumpResult)
		SB_LOG_CRITICAL("Failed to create crash dump! {}", GetResultDescription(GetLastError()));
	CloseHandle(FileHandle);
	if (ExceptionInfo->ExceptionRecord->NumberParameters == 2)
	{
		HRESULT Code = static_cast<HRESULT>(ExceptionInfo->ExceptionRecord->ExceptionInformation[0]);
		const char* Message = reinterpret_cast<const char*>(ExceptionInfo->ExceptionRecord->ExceptionInformation[1]);
		SB_LOG_CRITICAL("Application crashed with code {} ({})", Code, GetResultDescription(Code));
		SB_LOG_CRITICAL("{}", Message);
		MessageBox(nullptr, std::format(
			           "An error occurred. A crash dump has been created.\n\n{}\n\nCode: {} ({})\nProcess: {}\nThread: {}",
			           Message, Code, GetResultDescription(Code),
			           GetCurrentProcessId(), GetCurrentThreadId()).c_str(),
		           TEXT("Snowbite"), MB_OK | MB_ICONERROR);
	}
	else
	{
		SB_LOG_CRITICAL("Application crashed with code {} ({})", ExceptionInfo->ExceptionRecord->ExceptionCode,
		                GetResultDescription(ExceptionInfo->ExceptionRecord->ExceptionCode));
		MessageBox(nullptr, std::format(
			           "Unexpected error occurred. A crash dump has been created.\n\nCode: {} ({})\nProcess: {}\nThread: {}",
			           ExceptionInfo->ExceptionRecord->ExceptionCode,
			           GetResultDescription(ExceptionInfo->ExceptionRecord->ExceptionCode), GetCurrentProcessId(),
			           GetCurrentThreadId(),
			           reinterpret_cast<const char*>(ExceptionInfo->ExceptionRecord->ExceptionInformation[0])).c_str(),
		           TEXT("Snowbite"), MB_OK | MB_ICONERROR);
	}
	return EXCEPTION_CONTINUE_SEARCH;
}

// This function is called when the user presses Ctrl+C or closes the console window and allows the engine to gracefully shutdown
BOOL WINAPI ControlHandler(const DWORD ControlType)
{
	switch (ControlType)
	{
	case CTRL_C_EVENT:
	case CTRL_CLOSE_EVENT:
		GetEngine()->RequestShutdown();
		return TRUE;
	case CTRL_BREAK_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
	default:
		return FALSE;
	}
}

uint32_t LaunchSnowbite(const int ArgumentCount, char* Arguments[])
{
	SetUnhandledExceptionFilter(HandleException);
	SetConsoleCtrlHandler(ControlHandler, TRUE);
	FLogging::Initialize();
	FArgumentParser ArgumentParser(ArgumentCount, Arguments);
	FEngine::EngineInstance = std::make_shared<FEngine>(ArgumentParser);
	const HRESULT InitializeResult = GetEngine()->Initialize();
	if (SUCCEEDED(InitializeResult))
		GetEngine()->Run();
	const HRESULT ShutdownResult = GetEngine()->Shutdown(InitializeResult);
	SB_ASSERT_CRITICAL(GetEngine().use_count() <= 2, E_TOO_MUCH_REFERENCES, "Too much references to the engine");
	SB_SAFE_RESET(FEngine::EngineInstance);
	FLogging::Shutdown();
	return ShutdownResult;
}
