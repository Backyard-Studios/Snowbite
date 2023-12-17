#pragma once

#include <Engine/Core/Definitions.h>

#include <Engine/Graphics/DirectXInclude.h>

#include "BufferingMode.h"
#include "ClearColor.h"
#include "Fence.h"
#include "IndexBuffer.h"
#include "SwapChain.h"
#include "VertexBuffer.h"

struct SNOWBITE_API FGraphicsDeviceSettings
{
	bool bEnableDebugLayer = false;
	bool bEnableGPUValidation = false;
	D3D_FEATURE_LEVEL FeatureLevel = D3D_FEATURE_LEVEL_12_0;
	DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	EBufferingMode BufferingMode = EBufferingMode::TripleBuffering;
	std::shared_ptr<FWindow> Window;
};

struct SNOWBITE_API FDrawCall
{
	std::shared_ptr<FVertexBuffer> VertexBuffer;
	std::shared_ptr<FIndexBuffer> IndexBuffer;
};

struct SNOWBITE_API FConstantBuffer
{
	float Offset = 0.5f;
};

class SNOWBITE_API FGraphicsDevice
{
public:
	FGraphicsDevice(const FGraphicsDeviceSettings& InSettings);
	~FGraphicsDevice();

	void SignalAndWait() const;
	void Flush(uint32_t Count = 1) const;
	void ExecuteCommandList(ComPointer<ID3D12GraphicsCommandList7> CommandList);

	void Resize(uint32_t InWidth, uint32_t InHeight);

	std::shared_ptr<FVertexBuffer> CreateVertexBuffer(FVertex* Vertices, uint32_t Count);
	std::shared_ptr<FIndexBuffer> CreateIndexBuffer(FIndex* Indices, uint32_t Count);

	void BeginFrame(const FClearColor& ClearColor);
	void EndFrame();

	void Draw(const FDrawCall& DrawCall) const;

	[[nodiscard]] ComPointer<IDXGIFactory7> GetFactory() const { return Factory; }
	[[nodiscard]] ComPointer<ID3D12Device10> GetDevice() const { return Device; }
	[[nodiscard]] ComPointer<ID3D12CommandQueue> GetCommandQueue() const { return CommandQueue; }
	[[nodiscard]] ComPointer<ID3D12DescriptorHeap> GetRtvDescriptorHeap() const { return RtvDescriptorHeap; }
	[[nodiscard]] uint32_t GetRtvDescriptorSize() const { return RtvDescriptorSize; }

private:
	D3D12_SHADER_BYTECODE CompileShader(const std::wstring& FileName, const std::string& EntryPoint,
	                                    const std::string& Target, ComPointer<ID3DBlob>& ShaderBlob) const;
	void SetViewportAndScissor(uint32_t Width, uint32_t Height);

	void CreateDepthStencilBuffer(uint32_t Width, uint32_t Height);
	void ReleaseDepthStencilBuffer();

private:
	FGraphicsDeviceSettings Settings;
	uint32_t BufferCount;

	bool bIsResizing = false;

	ComPointer<IDXGIFactory7> Factory;

	ComPointer<ID3D12Debug6> DebugController;
	ComPointer<IDXGIDebug1> DXGIDebugController;

	ComPointer<ID3D12Device10> Device;
	ComPointer<ID3D12CommandQueue> CommandQueue;

	std::vector<ComPointer<ID3D12CommandAllocator>> CommandAllocators;
	std::vector<ComPointer<ID3D12GraphicsCommandList7>> CommandLists;
	std::vector<std::shared_ptr<FFence>> Fences;

	ComPointer<ID3D12DescriptorHeap> RtvDescriptorHeap;
	uint32_t RtvDescriptorSize = 0;

	ComPointer<ID3D12DescriptorHeap> SrvDescriptorHeap;
	uint32_t SrvDescriptorSize = 0;

	ComPointer<ID3D12DescriptorHeap> DsDescriptorHeap;
	uint32_t DsDescriptorSize = 0;

	FConstantBuffer ConstantBuffer = {};

	std::vector<ComPointer<ID3D12DescriptorHeap>> ConstantBufferDescriptorHeaps;
	std::vector<ComPointer<ID3D12Resource2>> ConstantBufferUploadHeaps;
	std::vector<uint8_t*> ConstantBufferUploadHeapPointers;

	ComPointer<ID3D12Resource2> DepthStencilBuffer;

	std::shared_ptr<FSwapChain> SwapChain;

	ComPointer<ID3D12RootSignature> RootSignature;
	ComPointer<ID3D12PipelineState> PipelineState;

	std::shared_ptr<FVertexBuffer> VertexBuffer;
	std::shared_ptr<FIndexBuffer> IndexBuffer;

	D3D12_VIEWPORT Viewport{};
	D3D12_RECT ScissorRect{};
};
