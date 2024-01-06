#pragma once

#define SB_MAKE_VERSION(major, minor, patch) \
	((((uint32_t)(major)) << 22U) | (((uint32_t)(minor)) << 12U) | ((uint32_t)(patch)))
#define SB_VERSION_MAJOR(version) ((uint32_t)(version) >> 22U)
#define SB_VERSION_MINOR(version) (((uint32_t)(version) >> 12U) & 0x3FFU)
#define SB_VERSION_PATCH(version) ((uint32_t)(version) & 0xFFFU)

// ReSharper disable CppClangTidyModernizeMacroToEnum
#define SNOWBITE_VERSION_MAJOR 0
#define SNOWBITE_VERSION_MINOR 0
#define SNOWBITE_VERSION_PATCH 0
#define SNOWBITE_VERSION_BRANCH "dev"
#define SNOWBITE_VERSION SB_MAKE_VERSION(SNOWBITE_VERSION_MAJOR, SNOWBITE_VERSION_MINOR, SNOWBITE_VERSION_PATCH)
// ReSharper enable CppClangTidyModernizeMacroToEnum

#ifdef SB_LIBRARY_EXPORT
#	define SNOWBITE_API __declspec(dllexport)
#else
#	define SNOWBITE_API __declspec(dllimport)
#endif

#define SB_UNIQUE_NAME_CONCAT_INNER(a, b) a ## b
#define SB_UNIQUE_NAME_CONCAT(a, b) SB_UNIQUE_NAME_CONCAT_INNER(a, b)

/**
 * Generates a unique name for a variable to be used in a macro.
 * @param prefix The prefix to use for the variable name.
 */
#define SB_UNIQUE_NAME(prefix) SB_UNIQUE_NAME_CONCAT(prefix, __LINE__)

#define SB_DEBUG_BREAK() if(IsDebuggerPresent()) { DebugBreak(); }

#define SB_SAFE_RELEASE(Pointer) if (Pointer) { (Pointer)->Release(); (Pointer) = nullptr; }
#define SB_SAFE_DELETE(Pointer) if (Pointer) { delete (Pointer); (Pointer) = nullptr; }
#define SB_SAFE_DELETE_ARRAY(Pointer) if (Pointer) { delete[] (Pointer); (Pointer) = nullptr; }
#define SB_SAFE_RESET(Pointer) if (Pointer) { (Pointer).reset(); (Pointer) = nullptr; }
#define SB_SAFE_COM_RESET(Pointer) if (Pointer) { (Pointer).Reset(); (Pointer) = nullptr; }
#define SB_SAFE_DESTROY(Pointer) if (Pointer) { (Pointer)->Destroy(); (Pointer).reset(); (Pointer) = nullptr; }

#define SB_CHECK(Result) { const HRESULT SB_UNIQUE_NAME(ResultVariable) = (Result); if (FAILED(SB_UNIQUE_NAME(ResultVariable))) { if(IsDebuggerPresent()) { FPlatform::PrintHRESULT(SB_UNIQUE_NAME(ResultVariable)); DebugBreak(); } return SB_UNIQUE_NAME(ResultVariable); } }

#define SB_RETURN_IF_FAILED(Result) if (FAILED(Result)) { return Result; }
