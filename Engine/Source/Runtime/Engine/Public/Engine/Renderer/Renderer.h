#pragma once

#include <Engine/Graphics/GraphicsInterface.h>

class FRenderer
{
public:
	[[nodiscard]] static HRESULT BeginFrame();
	[[nodiscard]] static HRESULT EndFrame();

private:
	[[nodiscard]] static HRESULT Initialize(const std::shared_ptr<FWindow>& InWindow);
	static void Shutdown();

private:
	static std::shared_ptr<FWindow> Window;
	static std::shared_ptr<IGraphicsInterface> GraphicsInterface;

	friend class FEngine;
};
