#pragma once

#include <Engine/Core/Definitions.h>
#include <Engine/Graphics/DirectXInclude.h>

struct SNOWBITE_API FVertexShaderOptions
{
	const char* EntryPoint = "main";
};

enum class SNOWBITE_API EShaderType
{
	Vertex,
	Pixel,
};

const char* GetShaderTypeString(EShaderType Type);

class SNOWBITE_API FShader
{
public:
	FShader(const char* ShaderFile, EShaderType Type, FVertexShaderOptions Options = {});
	~FShader();

	void ReleaseBlob();

	[[nodiscard]] ComPointer<ID3DBlob> GetShaderBlob() const { return ShaderBlob; }
	[[nodiscard]] D3D12_SHADER_BYTECODE GetBytecode() const { return ShaderBytecode; }

private:
	ComPointer<ID3DBlob> ShaderBlob;
	D3D12_SHADER_BYTECODE ShaderBytecode;
};
