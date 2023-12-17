// ReSharper disable CppDeprecatedEntity
// ReSharper disable CppClangTidyClangDiagnosticDeprecatedDeclarations
#pragma once

#include <Engine/Core/Definitions.h>
#include <codecvt>

template <typename Target>
struct SNOWBITE_API StringConverter
{
	std::basic_string<Target> operator()(std::basic_string_view<Target> src)
	{
		return std::basic_string<Target>(src);
	}
};

template <>
struct SNOWBITE_API StringConverter<char>
{
	std::basic_string<char> operator()(const std::basic_string_view<char> src) const
	{
		return std::basic_string(src);
	}

	std::basic_string<char> operator()(const std::basic_string_view<wchar_t> src) const
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.to_bytes(std::wstring(src));
	}
};

template <>
struct SNOWBITE_API StringConverter<wchar_t>
{
	std::basic_string<wchar_t> operator()(const std::basic_string_view<char> src) const
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.from_bytes(std::string(src));
	}

	std::basic_string<wchar_t> operator()(const std::basic_string_view<wchar_t> src) const
	{
		return std::basic_string(src);
	}
};
