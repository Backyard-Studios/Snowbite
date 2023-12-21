// ReSharper disable CppDeprecatedEntity
// ReSharper disable CppClangTidyClangDiagnosticDeprecatedDeclarations
#pragma once

#include <Engine/Core/Definitions.h>

SNOWBITE_API const char* WideCharToString(const wchar_t* Source, uint32_t Size = 128);
SNOWBITE_API wchar_t* StringToWideChar(const char* Source, uint32_t Size = 128);
