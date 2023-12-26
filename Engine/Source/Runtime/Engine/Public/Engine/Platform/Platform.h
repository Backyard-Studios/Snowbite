#pragma once

#include <cstdint>
#include <source_location>

class FPlatform
{
public:
	static HRESULT Initialize(HINSTANCE Instance);
	static void Shutdown();

	static void Fatal(HRESULT Code = E_FAIL);
	static void Fatal(const std::string& Message, HRESULT Code = E_FAIL);

	static void SetHighDpiAwareness(bool bIsAware);

	static LONG CALLBACK SehExceptionHandler(EXCEPTION_POINTERS* ExceptionPointers);
	static void CollectCrashInfo(LPEXCEPTION_POINTERS ExceptionPointers, DWORD ThreadId);

	static void PrintHRESULT(HRESULT Code, const std::source_location& Location = std::source_location::current());

	static bool IsMainThread() { return MainThreadId == GetCurrentThreadId(); }
	static bool IsMainThread(const HANDLE ThreadHandle) { return MainThreadId == GetThreadId(ThreadHandle); }
	static bool IsMainThread(const DWORD ThreadId) { return MainThreadId == ThreadId; }

	[[nodiscard]] static uint32_t GetDpiScale() { return DpiScale; }
	[[nodiscard]] static WNDCLASSEX GetWindowClass() { return WindowClass; }

private:
	static uint32_t CalculateDpiScale(HMODULE ShCoreModule);
	static LRESULT CALLBACK WindowProc(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam);

private:
	static uint32_t DpiScale;
	static WNDCLASSEX WindowClass;
	static DWORD MainThreadId;
};
