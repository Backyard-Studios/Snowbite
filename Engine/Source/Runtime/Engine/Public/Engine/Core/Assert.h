#pragma once

#include <Engine/Core/Definitions.h>
#include <source_location>

SNOWBITE_API ULONG_PTR* FormatExceptionArguments(HRESULT code, const char* message,
                                                 const std::source_location& location =
	                                                 std::source_location::current());

#ifdef SB_DEBUG
#	define SB_ASSERT(x, code, msg) { if(!(x)) { RaiseException(code, 0, 3, FormatExceptionArguments(code, msg)); } }
#else
#	define SB_ASSERT(x, code)
#endif

#define SB_ASSERT_CRITICAL(x, code, msg) { if(!(x)) { RaiseException(code, 0, 3, FormatExceptionArguments(code, msg)); } }
