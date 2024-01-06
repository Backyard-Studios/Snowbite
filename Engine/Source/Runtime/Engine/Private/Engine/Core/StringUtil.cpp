#include "pch.h"

#include <Engine/Core/StringUtil.h>

std::string WideStringToString(const std::wstring& InWideString)
{
	const int Size = WideCharToMultiByte(CP_UTF8, 0, &InWideString[0], static_cast<int>(InWideString.size()), nullptr,
	                                     0, nullptr, nullptr);
	std::string Result(Size, 0);
	WideCharToMultiByte(CP_UTF8, 0, &InWideString[0], static_cast<int>(InWideString.size()), &Result[0], Size, nullptr,
	                    nullptr);
	return Result;
}

std::wstring StringToWideString(const std::string& InString)
{
	const int Size = MultiByteToWideChar(CP_UTF8, 0, &InString[0], static_cast<int>(InString.size()), nullptr, 0);
	std::wstring Result(Size, 0);
	MultiByteToWideChar(CP_UTF8, 0, &InString[0], static_cast<int>(InString.size()), &Result[0], Size);
	return Result;
}
