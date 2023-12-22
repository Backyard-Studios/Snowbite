#pragma once

#include <Engine/Core/Definitions.h>

#include "BufferingMode.h"
#include "ClearColor.h"
#include "RHI.h"
#include "Window.h"

struct SNOWBITE_API FRendererSettings
{
	ERHIType RHIType = ERHIType::D3D12;
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
	void SetVSync(bool bInIsVSyncEnabled);

	/**
	 * @brief Begins a new frame.
	 * Must be called before any rendering commands are issued.
	 * @return S_OK if successful, otherwise an error code.
	 */
	HRESULT BeginFrame() const;
	/**
	 * @brief Ends the current frame.
	 * Must be called after all rendering commands have been issued.
	 * @return S_OK if successful, otherwise an error code.
	 */
	HRESULT EndFrame() const;

	void BeginUIFrame() const;
	void EndUIFrame() const;

	[[nodiscard]] ERHIType GetRHIType() const { return Settings.RHIType; }
	[[nodiscard]] const char* GetRHIName() const { return RHI->GetName(); }

	[[nodiscard]] bool IsVSyncEnabled() const { return bIsVSyncEnabled; }

private:
	FRendererSettings Settings;

	std::shared_ptr<IStaticRHI> RHI;
	FClearColor ClearColor;
	bool bIsVSyncEnabled = false;
};
