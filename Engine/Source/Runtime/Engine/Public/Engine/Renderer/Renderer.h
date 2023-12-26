#pragma once

#include <Engine/Graphics/GraphicsInterface.h>

class FRenderer
{
public:
	[[nodiscard]] static HRESULT BeginFrame();
	[[nodiscard]] static HRESULT EndFrame();

private:
	[[nodiscard]] static HRESULT Initialize();
	static void Shutdown();

private:
	static std::shared_ptr<IGraphicsInterface> GraphicsInterface;

	friend class FEngine;
};
