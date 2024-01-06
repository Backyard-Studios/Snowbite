#pragma once

#include <Engine/Core/Definitions.h>
#include <combaseapi.h>

#include "StringUtil.h"

/**
 * Thin wrapper around Guid
 */
struct SNOWBITE_API FIdentifier
{
public:
	FIdentifier()
	{
		CoCreateGuid(&Guid);
	}

	FIdentifier(const FIdentifier& Other)
	{
		Guid = Other.Guid;
	}

	FIdentifier& operator=(const FIdentifier& Other)
	{
		Guid = Other.Guid;
		return *this;
	}

	friend bool operator==(const FIdentifier& Lhs, const FIdentifier& Rhs)
	{
		return Lhs.Guid == Rhs.Guid;
	}

	friend bool operator!=(const FIdentifier& Lhs, const FIdentifier& Rhs)
	{
		return !(Lhs == Rhs);
	}

	operator GUID() const
	{
		return Guid;
	}

	operator GUID()
	{
		return Guid;
	}

	operator const GUID*() const
	{
		return &Guid;
	}

	operator GUID*()
	{
		return &Guid;
	}

private:
	GUID Guid;
};

SNOWBITE_API inline std::string ToString(const FIdentifier& Identifier)
{
	OLECHAR* GuidString;
	StringFromCLSID(Identifier, &GuidString);
	const std::wstring GuidWString = GuidString;
	CoTaskMemFree(GuidString);
	return WideStringToString(GuidWString);
}
