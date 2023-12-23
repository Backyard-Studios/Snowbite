#include "pch.h"

#include <Engine/Engine.h>

/**
 * Calls the specified lifecycle method. Only use this macro if the lifecycle method returns a result.
 * @param Method The lifecycle method to call.
 * @param ... The arguments to pass to the lifecycle method.
 */
#define SB_LIFECYCLE_WITH_RESULT(Method, ...) \
	const HRESULT SB_UNIQUE_NAME(Result) = Method(__VA_ARGS__); \
	if (FAILED(SB_UNIQUE_NAME(Result))) \
	{ \
		return SB_UNIQUE_NAME(Result); \
	}

/**
 * Only exists to distinguish between normal methods and lifecycle methods.
 * Calls the specified lifecycle method. Only use this macro if the lifecycle method does *not* return a result.
 * @param Method The lifecycle method to call.
 * @param ... The arguments to pass to the lifecycle method.
 */
#define SB_LIFECYCLE(Method, ...) \
	Method(__VA_ARGS__);

#define SB_SERVICES_LIFECYCLE_WITH_RESULT(Method, ...) \
	for (const std::shared_ptr<IEngineService>& EngineService : EngineServices) \
	{ \
		const HRESULT Result = EngineService->Method(__VA_ARGS__); \
		if (FAILED(Result)) \
			return Result; \
	}

#define SB_SERVICES_LIFECYCLE(Method, ...) \
	for (const std::shared_ptr<IEngineService>& EngineService : EngineServices) \
	{ \
		EngineService->Method(__VA_ARGS__); \
	}

bool FEngine::bShouldExit = false;
uint32_t FEngine::ExitCode = EXIT_SUCCESS;
std::vector<std::shared_ptr<IEngineService>> FEngine::EngineServices;
std::shared_ptr<FWindow> FEngine::MainWindow;

uint32_t FEngine::EntryPoint(int ArgumentCount, char* ArgumentArray[])
{
	// Register engine services
	RegisterService<FWindowManagerService>();

	SB_LIFECYCLE_WITH_RESULT(PreInitialize)

	SB_LIFECYCLE_WITH_RESULT(Initialize)

	SB_LIFECYCLE_WITH_RESULT(PostInitialize)
	while (!bShouldExit)
	{
		SB_LIFECYCLE(EarlyUpdate)

		SB_LIFECYCLE(Update)

		SB_LIFECYCLE(LateUpdate)
	}
	SB_LIFECYCLE(BeforeShutdown)

	SB_LIFECYCLE(Shutdown)

	SB_LIFECYCLE(AfterShutdown)

	// Unregister engine services
	for (std::shared_ptr<IEngineService>& EngineService : EngineServices)
		EngineService.reset();
	EngineServices.clear();
	return ExitCode;
}

void FEngine::RequestExit(const uint32_t InExitCode)
{
	ExitCode = InExitCode;
	bShouldExit = true;
}

void FEngine::Exit(const uint32_t InExitCode)
{
	std::exit(InExitCode);
}

void FEngine::RegisterService(const std::shared_ptr<IEngineService>& EngineService)
{
	EngineServices.push_back(EngineService);
}

HRESULT FEngine::PreInitialize()
{
	SB_SERVICES_LIFECYCLE_WITH_RESULT(PreInitialize)
	return S_OK;
}

HRESULT FEngine::Initialize()
{
	SB_SERVICES_LIFECYCLE_WITH_RESULT(Initialize)
	FWindowDesc WindowDesc;
	WindowDesc.bShouldRequestExitOnClose = true;
	WindowDesc.bShouldAutoShow = false;
	MainWindow = std::make_shared<FWindow>(WindowDesc);
	FWindowManager::Register(MainWindow);

	FWindowDesc SecondWindowDesc;
	SecondWindowDesc.Parent = MainWindow;
	SecondWindowDesc.Title = TEXT("Second Window");
	SecondWindowDesc.Size = {800, 600};
	SecondWindowDesc.bShouldUnregisterOnClose = true;
	FWindowManager::Register(std::make_shared<FWindow>(SecondWindowDesc));
	return S_OK;
}

HRESULT FEngine::PostInitialize()
{
	SB_SERVICES_LIFECYCLE_WITH_RESULT(PostInitialize)
	MainWindow->Show();
	MainWindow->Flash();
	return S_OK;
}

void FEngine::EarlyUpdate()
{
	SB_SERVICES_LIFECYCLE(EarlyUpdate)
}

void FEngine::Update()
{
	SB_SERVICES_LIFECYCLE(Update)
}

void FEngine::LateUpdate()
{
	SB_SERVICES_LIFECYCLE(LateUpdate)
}

void FEngine::BeforeShutdown()
{
	SB_SERVICES_LIFECYCLE(BeforeShutdown)
}

void FEngine::Shutdown()
{
	FWindowManager::Unregister(MainWindow);
	MainWindow.reset();
	MainWindow = nullptr;

	SB_SERVICES_LIFECYCLE(Shutdown)
}

void FEngine::AfterShutdown()
{
	SB_SERVICES_LIFECYCLE(AfterShutdown)
}
