#pragma once

#include <Engine/Core/Definitions.h>

#include <memory>

#include "Core/ArgumentParser.h"
#include "Graphics/Window.h"

// ReSharper disable once CppClangTidyCertDcl58Cpp
SB_EXPORT_STL_CONTAINER(std::shared_ptr, FEngine)

class SNOWBITE_API FEngine
{
public:
	FEngine(const FArgumentParser& InArgumentParser);
	~FEngine() = default;
	SB_DISABLE_COPY_AND_MOVE(FEngine)

	void RequestShutdown();

	[[nodiscard]] bool IsShutdownRequested() const { return bIsShutdownRequested; }
	[[nodiscard]] bool IsHeadless() const { return bIsHeadless; }

private:
	HRESULT Initialize();
	/**
	 * Starts the update loop
	 */
	void Run();
	HRESULT Shutdown(HRESULT ExitCode = S_OK);

private:
	inline static std::shared_ptr<FEngine> EngineInstance = nullptr;

private:
	FArgumentParser ArgumentParser;
	bool bIsShutdownRequested = false;
	bool bIsHeadless = false;

	std::shared_ptr<FWindow> MainWindow = nullptr;

	friend SNOWBITE_API uint32_t LaunchSnowbite(int ArgumentCount, char* Arguments[]);
	friend SNOWBITE_API std::shared_ptr<FEngine> GetEngine();
};

/**
 * Returns the engine instance
 */
SNOWBITE_API std::shared_ptr<FEngine> GetEngine();
