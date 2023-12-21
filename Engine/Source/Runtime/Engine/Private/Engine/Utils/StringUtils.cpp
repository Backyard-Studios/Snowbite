#include "pch.h"

#include <Engine/Utils/StringUtils.h>

const char* WideCharToString(const wchar_t* Source, const uint32_t Size)
{
	char* Buffer = new char[Size];
	WideCharToMultiByte(CP_ACP, 0, Source, -1, Buffer, Size, nullptr, nullptr);
	return Buffer;
}

wchar_t* StringToWideChar(const char* Source, const uint32_t Size)
{
	wchar_t* Buffer = new wchar_t[Size];
	MultiByteToWideChar(CP_ACP, 0, Source, -1, Buffer, Size);
	return Buffer;
}
