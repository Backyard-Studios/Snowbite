#include "pch.h"

#include <Engine/Engine.h>

std::shared_ptr<FEngine> GetEngine()
{
	return FEngine::EngineInstance;
}
