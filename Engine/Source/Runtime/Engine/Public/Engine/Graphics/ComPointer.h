// ReSharper disable CppClangTidyCppcoreguidelinesRvalueReferenceParamNotMoved
// ReSharper disable CppClangTidyMiscUnconventionalAssignOperator
// ReSharper disable CppClangTidyGoogleRuntimeOperator
#pragma once

/*
	This is free and unencumbered software released into the public domain.

	Anyone is free to copy, modify, publish, use, compile, sell, or
	distribute this software, either in source code form or as a compiled
	binary, for any purpose, commercial or non-commercial, and by any
	means.

	In jurisdictions that recognize copyright laws, the author or authors
	of this software dedicate any and all copyright interest in the
	software to the public domain. We make this dedication for the benefit
	of the public at large and to the detriment of our heirs and
	successors. We intend this dedication to be an overt act of
	relinquishment in perpetuity of all present and future rights to this
	software under copyright law.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
	OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
	ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
	OTHER DEALINGS IN THE SOFTWARE.

	For more information, please refer to <http://unlicense.org/>

	https://gist.github.com/Ohjurot/e17a5f04e9719a44866b4f38a4f2f680
	Created by Ludwig Fuechsl
	Last Change: 07.04.2023
*/

// ReSharper disable once CppUnusedIncludeDirective
#include <Engine/Graphics/DirectXInclude.h>
#include <type_traits>


/// <summary>
/// A template class for Microsoft com pointer (I like this one more the the WRL pointer)
/// </summary>
/// <typeparam name="T">Com pointer Type</typeparam>
template <typename CT, typename = std::enable_if_t<std::is_base_of_v<IUnknown, CT>>>
class ComPointer
{
public:
	// Default empty constructor
	ComPointer() = default;

	// Construct by raw pointer (add ref)
	ComPointer(CT* InPointer)
	{
		SetPointerAndAddRef(InPointer);
	}

	ComPointer(const ComPointer<CT>& Other)
	{
		SetPointerAndAddRef(Other.Pointer);
	}

	explicit ComPointer(ComPointer<CT>&& Other) noexcept
	{
		Pointer = Other.Pointer;
		Other.Pointer = nullptr;
	}

	~ComPointer()
	{
		ClearPointer();
	}

	ComPointer<CT>& operator=(const ComPointer<CT>& Other)
	{
		ClearPointer();
		SetPointerAndAddRef(Other.Pointer);
		return *this;
	}

	ComPointer<CT>& operator=(ComPointer<CT>&& Other)
	{
		ClearPointer();
		Pointer = Other.Pointer;
		Other.Pointer = nullptr;
		return *this;
	}

	ComPointer<CT>& operator=(CT* Other)
	{
		ClearPointer();
		SetPointerAndAddRef(Other);
		return *this;
	}

	ULONG Release()
	{
		return ClearPointer();
	}

	CT* GetRef()
	{
		if (Pointer)
		{
			Pointer->AddRef();
			return Pointer;
		}
		return nullptr;
	}

	CT* Get()
	{
		return Pointer;
	}

	template <typename T>
	bool QueryInterface(ComPointer<T>& Other, HRESULT* ErrorCode = nullptr)
	{
		if (Pointer)
		{
			const HRESULT result = Pointer->QueryInterface(IID_PPV_ARGS(&Other));
			if (ErrorCode) *ErrorCode = result;
			return result == S_OK;
		}
		return false;
	}

	bool operator==(const ComPointer<CT>& Other)
	{
		return Pointer == Other.m_pointer;
	}

	bool operator==(const CT* Other)
	{
		return Pointer == Other;
	}

	CT* operator->()
	{
		return Pointer;
	}

	CT** operator&()
	{
		return &Pointer;
	}

	operator bool()
	{
		return Pointer != nullptr;
	}

	operator CT*()
	{
		return Pointer;
	}

private:
	ULONG ClearPointer()
	{
		ULONG NewRef = 0;
		if (Pointer)
		{
			NewRef = Pointer->Release();
			Pointer = nullptr;
		}
		return NewRef;
	}

	void SetPointerAndAddRef(CT* InPointer)
	{
		Pointer = InPointer;
		if (Pointer)
			Pointer->AddRef();
	}

private:
	CT* Pointer = nullptr;
};
