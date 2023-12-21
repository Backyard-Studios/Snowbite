#include "pch.h"

#include <Engine/Graphics/D3D12/D3D12Device.h>

#include "Engine/Graphics/D3D12/D3D12Utils.h"
#include "Engine/Utils/StringUtils.h"

FD3D12Device::FD3D12Device(const ComPointer<IDXGIFactory7>& InFactory, const FD3D12DeviceDesc& InDesc)
	: Factory(InFactory), Desc(InDesc), AdapterDesc({})
{
}

FD3D12Device::~FD3D12Device() = default;

HRESULT FD3D12Device::Initialize()
{
	Adapter = GetSuitableAdapter(Desc.AdapterRequirements);
	if (!Adapter)
		return E_D3D12_ADAPTER_NOT_FOUND;
	SB_LOG_INFO("Using '{}' adapter", WideCharToString(AdapterDesc.Description, 128));
	SB_LOG_INFO("\t- Dedicated video memory: {} MB", AdapterDesc.DedicatedVideoMemory / 1024 / 1024);
	SB_LOG_INFO("\t- Shared system memory: {} MB", AdapterDesc.SharedSystemMemory / 1024 / 1024);
	const HRESULT CreateDeviceResult = D3D12CreateDevice(Adapter, Desc.AdapterRequirements.MinimumFeatureLevel,
	                                                     IID_PPV_ARGS(&Device));
	if (FAILED(CreateDeviceResult))
		return CreateDeviceResult;
	SB_LOG_DEBUG("Created device with feature level {}",
	             D3D12Utils::FeatureLevelToString(Desc.AdapterRequirements.MinimumFeatureLevel));
	Device->SetName(StringToWideChar(Desc.Name, 128));
	ShaderModel = GetHighestSupportedShaderModel();
	SB_LOG_DEBUG("Using shader model {}", D3D12Utils::ShaderModelToString(ShaderModel));
	constexpr D3D12_FORMAT_SUPPORT1 RTVFlags = D3D12_FORMAT_SUPPORT1_DISPLAY | D3D12_FORMAT_SUPPORT1_RENDER_TARGET;
	if (!IsFormatSupported(RenderTargetViewFormat, RTVFlags))
	{
		SB_LOG_ERROR("RenderTargetViewFormat '{}' is not supported",
		             D3D12Utils::DXGIFormatToString(RenderTargetViewFormat));
		return E_D3D12_FORMAT_NOT_SUPPORTED;
	}
	constexpr D3D12_FORMAT_SUPPORT1 DSVFlags = D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL;
	if (!IsFormatSupported(DepthStencilViewFormat, DSVFlags))
	{
		SB_LOG_ERROR("DepthStencilViewFormat '{}' is not supported",
		             D3D12Utils::DXGIFormatToString(DepthStencilViewFormat));
		return E_D3D12_FORMAT_NOT_SUPPORTED;
	}
	D3D12MA::ALLOCATOR_DESC AllocatorDesc = {};
	AllocatorDesc.pDevice = Device;
	AllocatorDesc.pAdapter = Adapter;
	AllocatorDesc.Flags = D3D12MA::ALLOCATOR_FLAG_DEFAULT_POOLS_NOT_ZEROED;
	const HRESULT CreateAllocatorResult = CreateAllocator(&AllocatorDesc, &Allocator);
	SB_D3D_ASSERT_RETURN(CreateAllocatorResult, "Failed to create allocator");
	return S_OK;
}

void FD3D12Device::Destroy()
{
	Allocator.Release();
	Device.Release();
	Adapter.Release();
}

D3D_SHADER_MODEL FD3D12Device::GetHighestSupportedShaderModel(const D3D_SHADER_MODEL PreferredShaderModel)
{
	D3D12_FEATURE_DATA_SHADER_MODEL ShaderModelFeature = {PreferredShaderModel};
	const HRESULT CheckFeatureSupportResult = Device->CheckFeatureSupport(
		D3D12_FEATURE_SHADER_MODEL, &ShaderModelFeature,
		sizeof(ShaderModelFeature));
	SB_D3D_ASSERT(CheckFeatureSupportResult, "Unable to check feature support");
	return ShaderModelFeature.HighestShaderModel;
}

bool FD3D12Device::IsFormatSupported(const DXGI_FORMAT Format, const D3D12_FORMAT_SUPPORT1 Support1,
                                     const D3D12_FORMAT_SUPPORT2 Support2)
{
	D3D12_FEATURE_DATA_FORMAT_SUPPORT FormatSupport = {Format};
	const HRESULT CheckFeatureSupportResult = Device->CheckFeatureSupport(
		D3D12_FEATURE_FORMAT_SUPPORT, &FormatSupport,
		sizeof(FormatSupport));
	SB_D3D_ASSERT(CheckFeatureSupportResult, "Unable to check feature support");
	return (FormatSupport.Support1 & Support1) == Support1 && (FormatSupport.Support2 & Support2) == Support2;
}

HRESULT FD3D12Device::CreateCommandQueue(ComPointer<ID3D12CommandQueue>& CommandQueue,
                                         const D3D12_COMMAND_LIST_TYPE Type,
                                         const char* Name)
{
	D3D12_COMMAND_QUEUE_DESC CommandQueueDesc = {};
	CommandQueueDesc.Type = Type;
	CommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	const HRESULT CreateCommandQueueResult = Device->CreateCommandQueue(&CommandQueueDesc, IID_PPV_ARGS(&CommandQueue));
	if (SUCCEEDED(CreateCommandQueueResult))
		CommandQueue->SetName(StringToWideChar(Name));
	return CreateCommandQueueResult;
}

std::shared_ptr<FD3D12CommandList> FD3D12Device::CreateCommandList(D3D12_COMMAND_LIST_TYPE Type, const char* Name)
{
	return std::make_shared<FD3D12CommandList>(Device, Type, Name);
}

std::shared_ptr<FD3D12Fence> FD3D12Device::CreateFence(const char* Name)
{
	return std::make_shared<FD3D12Fence>(Device, Name);
}

ComPointer<IDXGIAdapter4> FD3D12Device::GetSuitableAdapter(const FD3D12AdapterRequirements& Requirements)
{
	ComPointer<IDXGIAdapter4> TempAdapter;
	uint32_t Index = 0;
	while (Factory->EnumAdapterByGpuPreference(Index, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
	                                           IID_PPV_ARGS(&TempAdapter)) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC1 TempAdapterDesc;
		TempAdapter->GetDesc1(&TempAdapterDesc);
		if (Requirements.bShouldBeHardwareAdapter && !(TempAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE))
		{
			if (SUCCEEDED(
				D3D12CreateDevice(TempAdapter, Requirements.MinimumFeatureLevel, _uuidof(ID3D12Device),nullptr)))
			{
				AdapterDesc = TempAdapterDesc;
				return TempAdapter;
			}
		}
		TempAdapter.Release();
		Index++;
	}
	SB_LOG_WARN("Failed to find suitable adapter");
	return nullptr;
}
