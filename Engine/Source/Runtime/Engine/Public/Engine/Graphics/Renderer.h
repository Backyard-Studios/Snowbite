#pragma once

#include <Engine/Core/Definitions.h>

#include "BufferingMode.h"
#include "ClearColor.h"
#include "GraphicsDevice.h"

struct SNOWBITE_API FRendererSettings
{
	EBufferingMode BufferingMode = EBufferingMode::TripleBuffering;
	std::shared_ptr<FWindow> Window;
};

class SNOWBITE_API FRenderer
{
public:
	FRenderer(const FRendererSettings& InSettings);
	~FRenderer();

	void Resize(uint32_t InWidth, uint32_t InHeight) const;
	void SetClearColor(FClearColor InClearColor);

	void BeginFrame() const;
	void EndFrame() const;

private:
	FRendererSettings Settings;
	std::shared_ptr<FGraphicsDevice> GraphicsDevice;
	FClearColor ClearColor;
};
