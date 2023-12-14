#include "pch.h"

#include <Engine/Engine.h>

HRESULT FEngine::Initialize() const
{
	return S_OK;
}

void FEngine::Run() const
{
}

HRESULT FEngine::Shutdown(const HRESULT exitCode) const
{
	return exitCode;
}

std::shared_ptr<FEngine> GetEngine()
{
	return FEngine::EngineInstance;
}
