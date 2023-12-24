#pragma once

#include <Engine/Graphics/GraphicsInterface.h>

class FRenderer
{
public:

private:
	static HRESULT Initialize();
	static void Shutdown();

private:
	static std::shared_ptr<IGraphicsInterface> GraphicsInterface;

	friend class FEngine;
};
