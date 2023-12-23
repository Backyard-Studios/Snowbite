#pragma once

#include <cstdint>

class FPlatform
{
public:
	static HRESULT Initialize();
	static void Shutdown();

	static void SetHighDpiAwareness(bool bIsAware);

	static LONG CALLBACK SehExceptionHandler(EXCEPTION_POINTERS* ExceptionPointers);
	static void CollectCrashInfo(LPEXCEPTION_POINTERS ExceptionPointers, DWORD ThreadId);

	[[nodiscard]] static uint32_t GetDpiScale() { return DpiScale; }

private:
	static uint32_t CalculateDpiScale(HMODULE ShCoreModule);

private:
	inline static uint32_t DpiScale = 0;
};
