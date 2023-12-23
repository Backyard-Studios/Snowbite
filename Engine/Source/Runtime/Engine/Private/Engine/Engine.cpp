#include "pch.h"

#include <Engine/Engine.h>

#define SB_CALL_SERVICES_WITH_RESULT(Method, ...) \
	for (const std::shared_ptr<IEngineService>& EngineService : EngineServices) \
	{ \
		const HRESULT Result = EngineService->Method(__VA_ARGS__); \
		if (FAILED(Result)) \
			return Result; \
	}

#define SB_CALL_SERVICES(Method, ...) \
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

	PreInitialize();

	Initialize();

	PostInitialize();
	while (!bShouldExit)
	{
		EarlyUpdate();

		Update();

		LateUpdate();
	}
	BeforeShutdown();

	Shutdown();

	AfterShutdown();

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
	SB_CALL_SERVICES_WITH_RESULT(PreInitialize)
	return S_OK;
}

HRESULT FEngine::Initialize()
{
	SB_CALL_SERVICES_WITH_RESULT(Initialize)
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
	SB_CALL_SERVICES_WITH_RESULT(PostInitialize)
	MainWindow->Show();
	MainWindow->Flash();
	return S_OK;
}

void FEngine::EarlyUpdate()
{
	SB_CALL_SERVICES(EarlyUpdate)
}

void FEngine::Update()
{
	SB_CALL_SERVICES(Update)
}

void FEngine::LateUpdate()
{
	SB_CALL_SERVICES(LateUpdate)
}

void FEngine::BeforeShutdown()
{
	SB_CALL_SERVICES(BeforeShutdown)
}

void FEngine::Shutdown()
{
	FWindowManager::Unregister(MainWindow);
	MainWindow.reset();
	MainWindow = nullptr;

	SB_CALL_SERVICES(Shutdown)
}

void FEngine::AfterShutdown()
{
	SB_CALL_SERVICES(AfterShutdown)
}
