#pragma once

#include <Engine/Core/Definitions.h>

#include <source_location>

#ifdef SB_DEBUG
/**
 * Asserts when the condition is false. This is a debug assert, meaning it will only be checked in debug builds.
 * @param Condition The condition to check.
 * @param Code The code to raise if the condition is false.
 * @param Message The message to raise if the condition is false.
 */
#	define SB_ASSERT(Condition, Code, Message) { if(!(Condition)) { ULONG_PTR Arguments[2] = { static_cast<ULONG_PTR>(Code), reinterpret_cast<ULONG_PTR>(Message) }; RaiseException(Code, 0, 2, Arguments); } }
#else
/**
 * Asserts when the condition is false. This is a debug assert, meaning it will only be checked in debug builds.
 * @param Condition The condition to check.
 * @param Code The code to raise if the condition is false.
 * @param Message The message to raise if the condition is false.
 */
#	define SB_ASSERT(x, code)
#endif

/**
 * Asserts when the condition is false. This is a critical assert, meaning it will always be checked.
 * @param Condition The condition to check.
 * @param Code The code to raise if the condition is false.
 * @param Message The message to raise if the condition is false.
 */
#define SB_ASSERT_CRITICAL(Condition, Code, Message) { if(!(Condition)) { ULONG_PTR Arguments[2] = { static_cast<ULONG_PTR>(Code), reinterpret_cast<ULONG_PTR>(Message) }; RaiseException(Code, 0, 2, Arguments); } }
