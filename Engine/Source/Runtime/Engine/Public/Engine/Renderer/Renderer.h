#pragma once

#include <Engine/Core/Definitions.h>

#include "GraphicsDevice.h"

class SNOWBITE_API FRenderer
{
public:
	[[nodiscard]] static HRESULT BeginFrame();
	[[nodiscard]] static HRESULT EndFrame();

private:
	[[nodiscard]] static HRESULT Initialize(const std::shared_ptr<FWindow>& Window);
	static void Shutdown();

	[[nodiscard]] static HRESULT Resize(uint32_t NewWidth, uint32_t NewHeight);

private:
	static std::shared_ptr<FGraphicsDevice> GraphicsDevice;

	friend class FEngine;
};
