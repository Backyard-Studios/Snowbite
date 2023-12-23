#pragma once
#include "Core/EngineService.h"

class FEngine
{
public:
	/**
	 * @brief The entry point of the engine.
	 * @param ArgumentCount The number of arguments passed to the engine.
	 * @param ArgumentArray The array of arguments passed to the engine.
	 * @return The exit code of the engine.
	 */
	static uint32_t EntryPoint(int ArgumentCount, char* ArgumentArray[]);

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
	/**
	 * @brief Registers an engine service.
	 * @param EngineService The engine service to register. The engine will *not* take ownership of the service.
	 */
	static void RegisterService(const std::shared_ptr<IEngineService>& EngineService);

	/**
	 * @brief Registers an engine service.
	 * @tparam T The type of the engine service to register. The engine will take ownership of the service.
	 */
	template <typename T>
	static void RegisterService()
	{
		RegisterService(std::make_shared<T>());
	}

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

	static std::vector<std::shared_ptr<IEngineService>> EngineServices;

	static std::shared_ptr<FWindow> MainWindow;

	friend class FPlatform;
};
