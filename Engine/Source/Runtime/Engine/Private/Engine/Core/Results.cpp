﻿#include "pch.h"

#include <Engine/Core/Results.h>

const char* GetResultDescription(const HRESULT Result)
{
	switch (Result)
	{
	case S_OK: return "Success";
	case E_FAIL: return "Fail";
	case E_TOO_MUCH_REFERENCES: return "Too much references";
	default: return "Unknown";
	}
}
