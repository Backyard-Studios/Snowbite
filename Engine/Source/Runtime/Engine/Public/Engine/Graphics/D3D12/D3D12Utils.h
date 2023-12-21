#pragma once

#include <Engine/Core/Definitions.h>
#include <Engine/Core/Assert.h>
#include <Engine/Graphics/D3D12/D3D12Include.h>
// ReSharper disable once CppUnusedIncludeDirective
#include <format>

namespace D3D12Utils
{
	SNOWBITE_API const char* FeatureLevelToString(D3D_FEATURE_LEVEL FeatureLevel);
	SNOWBITE_API const char* ShaderModelToString(D3D_SHADER_MODEL ShaderModel);
	SNOWBITE_API const char* ShaderModelToMajorString(D3D_SHADER_MODEL ShaderModel);
	SNOWBITE_API const char* DXGIFormatToString(DXGI_FORMAT Format);
	SNOWBITE_API D3D12_VIEWPORT CreateViewport(uint32_t Width, uint32_t Height);
	SNOWBITE_API D3D12_RECT CreateScissorRect(uint32_t Width, uint32_t Height);
}

#define SB_D3D_FAILED_RETURN(Result) \
	if (FAILED(Result)) \
	{ \
		return Result; \
	}

#define SB_D3D_ASSERT_RETURN(Result, ...) \
	if (FAILED(Result)) \
	{ \
		SB_LOG_ERROR(__VA_ARGS__); \
		return Result; \
	}

#define SB_D3D_ASSERT(Result, ...) SB_ASSERT_CRITICAL(SUCCEEDED(Result), Result, std::format(__VA_ARGS__).c_str())
