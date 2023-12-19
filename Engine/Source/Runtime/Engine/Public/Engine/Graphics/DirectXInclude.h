#pragma once

#include <dxgi1_6.h>
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <d3dcompiler.h>
#include <dxgidebug.h>
#include <wrl.h>
#include <Engine/Graphics/ComPointer.h>
#include <Engine/Graphics/DirectX12Helpers.h>

#include <Engine/Core/Assert.h>

#define SB_D3D_ASSERT(Result, Message) SB_ASSERT_CRITICAL(SUCCEEDED(Result), Result, Message);
