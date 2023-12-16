#pragma once

#include <Engine/Core/Definitions.h>
#include <Engine/Graphics/DirectXInclude.h>

class FGraphicsDevice;

// TODO: Temporary location
struct SNOWBITE_API FVector3
{
	float X = 0.0f;
	float Y = 0.0f;
	float Z = 0.0f;
};

// TODO: Temporary location
struct SNOWBITE_API FVector4
{
	float X = 0.0f;
	float Y = 0.0f;
	float Z = 0.0f;
	float W = 1.0f;
};

struct SNOWBITE_API FVertex
{
	FVector3 Position;
	FVector4 Color;
};

class SNOWBITE_API FVertexBuffer
{
public:
	FVertexBuffer(FGraphicsDevice* InGraphicsDevice, FVertex* InVertices, uint32_t InCount);
	~FVertexBuffer();

	void Upload(ComPointer<ID3D12GraphicsCommandList7> CommandList);
	void SetName(const std::wstring& Name);

	[[nodiscard]] uint32_t GetCount() const { return Count; }
	[[nodiscard]] size_t GetSize() const { return Size; }
	[[nodiscard]] ComPointer<ID3D12Resource2> GetResource() const { return VertexBuffer; }
	[[nodiscard]] D3D12_VERTEX_BUFFER_VIEW GetView() const { return VertexBufferView; }

private:
	FGraphicsDevice* GraphicsDevice;
	uint32_t Count;
	uint32_t Size;

	ComPointer<ID3D12Resource2> VertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView{};

	ComPointer<ID3D12Resource> VertexBufferUploadHeap;
	D3D12_SUBRESOURCE_DATA SubresourceData = {};
};
