#pragma once

#include <Engine/Core/Definitions.h>

#include <memory>

#include "Application/Application.h"
#include "Application/LayerStack.h"
#include "Core/ArgumentParser.h"
#include "Graphics/Renderer.h"
#include "Graphics/Window.h"

// ReSharper disable once CppClangTidyCertDcl58Cpp
SB_EXPORT_STL_CONTAINER(std::shared_ptr, FEngine)

class SNOWBITE_API FEngine
{
public:
	FEngine(const FArgumentParser& InArgumentParser, std::shared_ptr<IApplication> InApplication);
	~FEngine() = default;
	SB_DISABLE_COPY_AND_MOVE(FEngine)

	/**
	 * @brief Requests the engine to shutdown
	 * The request will be processed at the end of the current update loop
	 */
	void RequestShutdown();

	[[nodiscard]] bool IsShutdownRequested() const { return bIsShutdownRequested; }
	[[nodiscard]] bool IsHeadless() const { return bIsHeadless; }

	[[nodiscard]] std::shared_ptr<FWindow> GetMainWindow() const { return MainWindow; }
	[[nodiscard]] std::shared_ptr<FRenderer> GetRenderer() const { return Renderer; }
	[[nodiscard]] std::shared_ptr<IApplication> GetApplication() const { return Application; }

private:
	HRESULT Initialize();
	/**
	 * @brief Starts the update loop
	 */
	void Run();
	HRESULT Shutdown(HRESULT ExitCode = S_OK);

private:
	inline static std::shared_ptr<FEngine> EngineInstance = nullptr;

private:
	FArgumentParser ArgumentParser;
	std::shared_ptr<IApplication> Application = nullptr;

	bool bIsShutdownRequested = false;
	bool bIsHeadless = false;

	/**
	 * This layer stack is used for engine specific debug overlays
	 */
	std::unique_ptr<FLayerStack> LayerStack = nullptr;

	std::shared_ptr<FWindow> MainWindow = nullptr;
	std::shared_ptr<FRenderer> Renderer = nullptr;

	friend SNOWBITE_API uint32_t LaunchSnowbite(int ArgumentCount, char* Arguments[], ImGuiContext* GlobalImGuiContext,
	                                            std::shared_ptr<IApplication> Application);
	friend SNOWBITE_API std::shared_ptr<FEngine> GetEngine();
};

/**
 * Returns the engine instance
 */
SNOWBITE_API std::shared_ptr<FEngine> GetEngine();
