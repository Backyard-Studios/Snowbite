#pragma once

#include <Engine/Core/Definitions.h>

class SNOWBITE_API FEngine
{
public:
	/**
	 * @brief The entry point of the engine.
	 * @param ArgumentCount The number of arguments passed to the engine.
	 * @param ArgumentArray The array of arguments passed to the engine.
	 * @return The exit code of the engine.
	 */
	static HRESULT EntryPoint(int ArgumentCount, char* ArgumentArray[]);

	/**
	 * @brief Requests the engine to exit.
	 * @param InExitCode The exit code to return.
	 */
	static void RequestExit(uint32_t InExitCode = EXIT_SUCCESS);
	/**
	 * @brief Abruptly exits the engine.
	 * @param InExitCode The exit code to return.
	 */
	static void Exit(uint32_t InExitCode = EXIT_FAILURE);

public:
	[[nodiscard]] static std::shared_ptr<FWindow> GetMainWindow() { return MainWindow; }

private:
#pragma region Lifecycle Functions
	static HRESULT PreInitialize();
	static HRESULT Initialize();
	static HRESULT PostInitialize();

	static void EarlyUpdate();
	static void Update();
	static void LateUpdate();

	static void BeforeShutdown();
	static void Shutdown();
	static void AfterShutdown();
#pragma endregion

private:
	static bool bShouldExit;
	static uint32_t ExitCode;

	static std::shared_ptr<FWindow> MainWindow;

	friend class FPlatform;
};
