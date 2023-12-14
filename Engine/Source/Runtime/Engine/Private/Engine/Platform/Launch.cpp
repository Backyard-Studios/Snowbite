#include "pch.h"

#include <Engine/Platform/Launch.h>

#include "Engine/Engine.h"

uint32_t LaunchSnowbite(int argc, char* argv[])
{
	FEngine::EngineInstance = std::make_shared<FEngine>();
	const HRESULT initializeResult = GetEngine()->Initialize();
	if (SUCCEEDED(initializeResult))
		GetEngine()->Run();
	const HRESULT shutdownResult = GetEngine()->Shutdown(initializeResult);
	if (GetEngine().use_count() > 2)
		return 1;
	SB_SAFE_RESET(FEngine::EngineInstance);
	return shutdownResult;
}
