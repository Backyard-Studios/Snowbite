#include "pch.h"

#include <Engine/Platform/Launch.h>

#include "Engine/Engine.h"

uint32_t LaunchSnowbite(int argc, char* argv[])
{
	FEngine::EngineInstance = std::make_shared<FEngine>();
	if (FEngine::EngineInstance.use_count() != 1)
		return 1;
	SB_SAFE_RESET(FEngine::EngineInstance);
	return 0;
}
