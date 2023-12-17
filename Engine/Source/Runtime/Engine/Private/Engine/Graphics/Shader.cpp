#include "pch.h"

#include <codecvt>
#include <Engine/Graphics/Shader.h>
#include <filesystem>

#include "Engine/Utils/StringUtils.h"

const char* GetShaderTypeString(const EShaderType Type)
{
	switch (Type)
	{
	case EShaderType::Vertex:
		return "vs_5_0";
	case EShaderType::Pixel:
		return "ps_5_0";
	default:
		SB_LOG_ERROR("Unknown shader type");
		return "";
	}
}

FShader::FShader(const char* ShaderFile, const EShaderType Type, const FVertexShaderOptions Options)
{
	ComPointer<ID3DBlob> ErrorBlob;
	const HRESULT CompileResult = D3DCompileFromFile(StringConverter<wchar_t>()(ShaderFile).c_str(),
	                                                 nullptr, nullptr, Options.EntryPoint,
	                                                 GetShaderTypeString(Type), 0, 0,
	                                                 &ShaderBlob,
	                                                 &ErrorBlob);
	if (FAILED(CompileResult))
	{
		if (ErrorBlob)
		{
			std::string ErrorString(static_cast<char*>(ErrorBlob->GetBufferPointer()));
			SB_LOG_ERROR("Failed to compile shader '{}': {}", std::filesystem::path(ShaderFile).filename().string(),
			             ErrorString);
		}
		else
		{
			SB_LOG_ERROR("Failed to compile shader '{}'", std::filesystem::path(ShaderFile).filename().string());
		}
	}
	SB_D3D_ASSERT(CompileResult, "Failed to compile shader");
	ShaderBytecode.BytecodeLength = ShaderBlob->GetBufferSize();
	ShaderBytecode.pShaderBytecode = ShaderBlob->GetBufferPointer();
}

FShader::~FShader()
{
	if (ShaderBlob)
		ShaderBlob.Release();
}

void FShader::ReleaseBlob()
{
	ShaderBlob.Release();
}
