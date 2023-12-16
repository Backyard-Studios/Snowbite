#pragma once

#include <Engine/Core/Definitions.h>
#include <Engine/Graphics/DirectXInclude.h>

class FGraphicsDevice;

SNOWBITE_API typedef uint32_t FIndex;

class SNOWBITE_API FIndexBuffer
{
public:
	FIndexBuffer(FGraphicsDevice* InGraphicsDevice, FIndex* InIndices, uint32_t InCount);
	~FIndexBuffer();

	void Upload(ComPointer<ID3D12GraphicsCommandList7> CommandList);
	void SetName(const std::wstring& Name);

	[[nodiscard]] uint32_t GetCount() const { return Count; }
	[[nodiscard]] uint32_t GetSize() const { return Size; }
	[[nodiscard]] ComPointer<ID3D12Resource2> GetResource() const { return IndexBuffer; }
	[[nodiscard]] D3D12_INDEX_BUFFER_VIEW GetView() const { return IndexBufferView; }

private:
	FGraphicsDevice* GraphicsDevice;
	uint32_t Count;
	uint32_t Size;

	ComPointer<ID3D12Resource2> IndexBuffer;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView{};

	ComPointer<ID3D12Resource> IndexBufferUploadHeap;
	D3D12_SUBRESOURCE_DATA IndexData = {};
};
