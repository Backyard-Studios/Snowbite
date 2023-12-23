#pragma once

#include <cstdint>

class FPlatform
{
public:
	static HRESULT Initialize(HINSTANCE Instance);
	static void Shutdown();

	static void SetHighDpiAwareness(bool bIsAware);

	static LONG CALLBACK SehExceptionHandler(EXCEPTION_POINTERS* ExceptionPointers);
	static void CollectCrashInfo(LPEXCEPTION_POINTERS ExceptionPointers, DWORD ThreadId);

	[[nodiscard]] static uint32_t GetDpiScale() { return DpiScale; }
	[[nodiscard]] static WNDCLASSEX GetWindowClass() { return WindowClass; }

private:
	static uint32_t CalculateDpiScale(HMODULE ShCoreModule);
	static LRESULT CALLBACK WindowProc(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam);

private:
	static uint32_t DpiScale;
	static WNDCLASSEX WindowClass;
};
