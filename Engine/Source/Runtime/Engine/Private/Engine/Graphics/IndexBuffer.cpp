#include "pch.h"

#include <Engine/Graphics/IndexBuffer.h>
#include <Engine/Graphics/GraphicsDevice.h>

FIndexBuffer::FIndexBuffer(FGraphicsDevice* InGraphicsDevice, FIndex* InIndices, const uint32_t InCount)
	: GraphicsDevice(InGraphicsDevice), Count(InCount), Size(sizeof(FIndex) * InCount)
{
	ComPointer<ID3D12Device10> Device = GraphicsDevice->GetDevice();

	const CD3DX12_RESOURCE_DESC ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(Size);

	const CD3DX12_HEAP_PROPERTIES HeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	const HRESULT IndexBufferCreateResult = Device->CreateCommittedResource(
		&HeapProperties, D3D12_HEAP_FLAG_NONE,
		&ResourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr,
		IID_PPV_ARGS(&IndexBuffer));
	SB_D3D_ASSERT(IndexBufferCreateResult, "Failed to create index buffer");

	const CD3DX12_HEAP_PROPERTIES UploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	const HRESULT IndexBufferUploadHeapCreateResult = Device->CreateCommittedResource(
		&UploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&ResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&IndexBufferUploadHeap));
	SB_D3D_ASSERT(IndexBufferUploadHeapCreateResult, "Failed to create index buffer upload heap");

	IndexData.pData = reinterpret_cast<BYTE*>(InIndices);
	IndexData.RowPitch = Size;
	IndexData.SlicePitch = Size;
}

FIndexBuffer::~FIndexBuffer()
{
	IndexBufferUploadHeap.Release();
	IndexBuffer.Release();
}

void FIndexBuffer::Upload(ComPointer<ID3D12GraphicsCommandList7> CommandList)
{
	UpdateSubresources(CommandList.Get(), IndexBuffer.Get(), IndexBufferUploadHeap.Get(), 0, 0, 1, &IndexData);
	const CD3DX12_RESOURCE_BARRIER IndexBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(IndexBuffer,
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	CommandList->ResourceBarrier(1, &IndexBufferBarrier);
	CommandList->Close();
	GraphicsDevice->ExecuteCommandList(CommandList);
	IndexBufferView.BufferLocation = IndexBuffer->GetGPUVirtualAddress();
	IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	IndexBufferView.SizeInBytes = Size;
}

void FIndexBuffer::SetName(const std::wstring& Name)
{
	IndexBuffer->SetName(Name.c_str());
	IndexBufferUploadHeap->SetName((Name + L" Upload Heap").c_str());
}
