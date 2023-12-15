#include "pch.h"

#include <Engine/Engine.h>

FEngine::FEngine(const FArgumentParser& InArgumentParser)
	: ArgumentParser(InArgumentParser)
{
}

void FEngine::RequestShutdown()
{
	bIsShutdownRequested = true;
}

HRESULT FEngine::Initialize()
{
	if (ArgumentParser.HasArgument("headless"))
		bIsHeadless = true;
	if (!IsHeadless())
	{
		FWindowDesc MainWindowDesc{};
		MainWindow = std::make_shared<FWindow>(MainWindowDesc);
		SB_ASSERT_CRITICAL(false, E_FAIL, "Failed to create main window");
	}
	return S_OK;
}

void FEngine::Run()
{
	while (!bIsShutdownRequested)
	{
		if (!IsHeadless())
		{
			MSG Message;
			if (PeekMessage(&Message, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&Message);
				DispatchMessage(&Message);
			}
			if (MainWindow->IsClosed())
				RequestShutdown();
		}
	}
}

HRESULT FEngine::Shutdown(const HRESULT ExitCode)
{
	if (!IsHeadless())
		SB_SAFE_RESET(MainWindow)
	return ExitCode;
}

std::shared_ptr<FEngine> GetEngine()
{
	return FEngine::EngineInstance;
}
