#include "pch.h"

#include <Engine/Graphics/VertexBuffer.h>
#include <Engine/Graphics/GraphicsDevice.h>

FVertexBuffer::FVertexBuffer(FGraphicsDevice* InGraphicsDevice, FVertex* InVertices, const uint32_t InCount)
	: GraphicsDevice(InGraphicsDevice), Count(InCount), Size(sizeof(FVertex) * InCount)
{
	ComPointer<ID3D12Device10> Device = InGraphicsDevice->GetDevice();

	const CD3DX12_RESOURCE_DESC ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(Size);

	const CD3DX12_HEAP_PROPERTIES HeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	const HRESULT VertexBufferCreateResult = Device->CreateCommittedResource(
		&HeapProperties, D3D12_HEAP_FLAG_NONE,
		&ResourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr,
		IID_PPV_ARGS(&VertexBuffer));
	SB_D3D_ASSERT(VertexBufferCreateResult, "Failed to create vertex buffer");

	const CD3DX12_HEAP_PROPERTIES UploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
	const HRESULT VertexBufferUploadHeapCreateResult = Device->CreateCommittedResource(
		&UploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&ResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&VertexBufferUploadHeap));
	SB_D3D_ASSERT(VertexBufferUploadHeapCreateResult, "Failed to create vertex buffer upload heap");
	VertexBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

	SubresourceData.pData = reinterpret_cast<BYTE*>(InVertices);
	SubresourceData.RowPitch = Size;
	SubresourceData.SlicePitch = Size;
}

FVertexBuffer::~FVertexBuffer()
{
	VertexBufferUploadHeap.Release();
	VertexBuffer.Release();
}

void FVertexBuffer::Upload(ComPointer<ID3D12GraphicsCommandList7> CommandList)
{
	UpdateSubresources(CommandList.Get(), VertexBuffer.Get(), VertexBufferUploadHeap.Get(), 0, 0, 1, &SubresourceData);
	const CD3DX12_RESOURCE_BARRIER Barrier = CD3DX12_RESOURCE_BARRIER::Transition(VertexBuffer,
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	CommandList->ResourceBarrier(1, &Barrier);
	CommandList->Close();
	GraphicsDevice->ExecuteCommandList(CommandList);
	VertexBufferView.BufferLocation = VertexBuffer->GetGPUVirtualAddress();
	VertexBufferView.StrideInBytes = sizeof(FVertex);
	VertexBufferView.SizeInBytes = Size;
}

void FVertexBuffer::SetName(const std::wstring& Name)
{
	VertexBuffer->SetName(Name.c_str());
	VertexBufferUploadHeap->SetName(Name.c_str());
}
