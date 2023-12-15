#include "pch.h"

#include <Engine/Core/Results.h>

const char* GetResultDescription(const HRESULT Result)
{
	switch (Result)
	{
	case S_OK: return "Success";
	case E_FAIL: return "Fail";
	case E_TOO_MUCH_REFERENCES: return "Too much references";
	case E_INVALIDARG: return "Invalid argument";
	case E_NOTIMPL: return "Not implemented";
	case S_FALSE: return "False";
	case E_OUTOFMEMORY: return "Out of memory";
	case DXGI_ERROR_WAS_STILL_DRAWING: return "DXGI error was still drawing";
	case D3D12_ERROR_DRIVER_VERSION_MISMATCH: return "D3D12 error driver version mismatch";
	case D3D12_ERROR_ADAPTER_NOT_FOUND: return "D3D12 error adapter not found";
	default: return "Unknown";
	}
}
