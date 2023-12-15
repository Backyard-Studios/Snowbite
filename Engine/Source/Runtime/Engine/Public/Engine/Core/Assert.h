#pragma once

#include <Engine/Core/Definitions.h>

#include <source_location>

#ifdef SB_DEBUG
#	define SB_ASSERT(Condition, Code, Message) { if(!(Condition)) { ULONG_PTR Arguments[2] = { static_cast<ULONG_PTR>(Code), reinterpret_cast<ULONG_PTR>(Message) }; RaiseException(Code, 0, 2, Arguments); } }
#else
#	define SB_ASSERT(x, code)
#endif

#define SB_ASSERT_CRITICAL(Condition, Code, Message) { if(!(Condition)) { ULONG_PTR Arguments[2] = { static_cast<ULONG_PTR>(Code), reinterpret_cast<ULONG_PTR>(Message) }; RaiseException(Code, 0, 2, Arguments); } }
