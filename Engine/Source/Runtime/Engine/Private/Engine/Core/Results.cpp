#include "pch.h"

#include <Engine/Core/Results.h>

const char* GetResultDescription(const HRESULT Result)
{
	switch (Result)
	{
	case S_OK: return "Success";
	case E_FAIL: return "Fail";
	case E_INVALIDARG: return "Invalid argument";
	case E_NOTIMPL: return "Not implemented";
	case S_FALSE: return "False";
	case E_OUTOFMEMORY: return "Out of memory";
	case D3D12_ERROR_DRIVER_VERSION_MISMATCH: return "D3D12 error driver version mismatch";
	case D3D12_ERROR_ADAPTER_NOT_FOUND: return "D3D12 error adapter not found";
	case DXGI_ERROR_ACCESS_DENIED: return "DXGI error access denied";
	case DXGI_ERROR_ACCESS_LOST: return "DXGI error access lost";
	case DXGI_ERROR_ALREADY_EXISTS: return "DXGI error already exists";
	case DXGI_ERROR_CANNOT_PROTECT_CONTENT: return "DXGI error cannot protect content";
	case DXGI_ERROR_DEVICE_HUNG: return "DXGI error device hung";
	case DXGI_ERROR_DEVICE_REMOVED: return "DXGI error device removed";
	case DXGI_ERROR_DEVICE_RESET: return "DXGI error device reset";
	case DXGI_ERROR_DRIVER_INTERNAL_ERROR: return "DXGI error driver internal error";
	case DXGI_ERROR_FRAME_STATISTICS_DISJOINT: return "DXGI error frame statistics disjoint";
	case DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE: return "DXGI error graphics vidpn source in use";
	case DXGI_ERROR_INVALID_CALL: return "DXGI error invalid call";
	case DXGI_ERROR_MORE_DATA: return "DXGI error more data";
	case DXGI_ERROR_NAME_ALREADY_EXISTS: return "DXGI error name already exists";
	case DXGI_ERROR_NONEXCLUSIVE: return "DXGI error nonexclusive";
	case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE: return "DXGI error not currently available";
	case DXGI_ERROR_NOT_FOUND: return "DXGI error not found";
	case DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED: return "DXGI error remote client disconnected";
	case DXGI_ERROR_REMOTE_OUTOFMEMORY: return "DXGI error remote out of memory";
	case DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE: return "DXGI error restrict to output stale";
	case DXGI_ERROR_SDK_COMPONENT_MISSING: return "DXGI error sdk component missing";
	case DXGI_ERROR_SESSION_DISCONNECTED: return "DXGI error session disconnected";
	case DXGI_ERROR_UNSUPPORTED: return "DXGI error unsupported";
	case DXGI_ERROR_WAIT_TIMEOUT: return "DXGI error wait timeout";
	case DXGI_ERROR_WAS_STILL_DRAWING: return "DXGI error was still drawing";

	// Custom errors
#pragma region General
	case E_TOO_MUCH_REFERENCES: return "Too much references";
#pragma endregion

#pragma region RHI
	case E_RHI_INITIALIZATION_FAILED: return "RHI initialization failed";
#pragma endregion

#pragma region RHI/D3D12
	case E_D3D12_ADAPTER_NOT_FOUND: return "D3D12 adapter not found";
	case E_D3D12_FORMAT_NOT_SUPPORTED: return "D3D12 format not supported";
#pragma endregion

	default: return "Unknown";
	}
}
