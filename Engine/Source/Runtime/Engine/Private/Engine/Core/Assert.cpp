#include "pch.h"

#include <Engine/Core/Assert.h>

ULONG_PTR* FormatExceptionArguments(const HRESULT code, const char* message, const std::source_location& location)
{
	ULONG_PTR arguments[3] = {};
	arguments[0] = static_cast<ULONG_PTR>(code);
	arguments[1] = reinterpret_cast<ULONG_PTR>(message);
	arguments[2] = reinterpret_cast<ULONG_PTR>(&location);
	return arguments;
}
