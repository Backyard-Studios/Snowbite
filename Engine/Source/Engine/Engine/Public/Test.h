#pragma once

#ifdef SB_LIBRARY_EXPORT
#	define SB_API __declspec(dllexport)
#else
#	define SB_API __declspec(dllimport)
#endif

#include <cstdint>

SB_API uint32_t TestFunction(uint32_t a, uint32_t b);
