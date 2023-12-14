#pragma once

#ifdef SB_LIBRARY_EXPORT
#	define SB_API __declspec(dllexport)
#else
#	define SB_API __declspec(dllimport)
#endif
