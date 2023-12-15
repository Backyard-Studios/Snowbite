#include "pch.h"

#include <Engine/Engine.h>

FEngine::FEngine(const FArgumentParser& InArgumentParser)
	: ArgumentParser(InArgumentParser)
{
}

HRESULT FEngine::Initialize() const
{
	return S_OK;
}

void FEngine::Run() const
{
}

HRESULT FEngine::Shutdown(const HRESULT ExitCode) const
{
	return ExitCode;
}

std::shared_ptr<FEngine> GetEngine()
{
	return FEngine::EngineInstance;
}
