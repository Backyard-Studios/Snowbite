#pragma once

#include "Engine/Graphics/CommandListType.h"
#include "Engine/Graphics/D3D12/D3D12Include.h"

inline D3D12_COMMAND_LIST_TYPE ConvertCommandListType(const ECommandListType Type)
{
	switch (Type)
	{
	case ECommandListType::Graphics: return D3D12_COMMAND_LIST_TYPE_DIRECT;
	case ECommandListType::Compute: return D3D12_COMMAND_LIST_TYPE_COMPUTE;
	default:
		FPlatform::Fatal("Invalid command list type");
		return D3D12_COMMAND_LIST_TYPE_NONE;
	}
}
