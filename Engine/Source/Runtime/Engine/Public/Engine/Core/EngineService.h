#pragma once

class IEngineService
{
public:
	virtual ~IEngineService() = default;

	virtual HRESULT PreInitialize() { return S_OK; }
	virtual HRESULT Initialize() { return S_OK; }
	virtual HRESULT PostInitialize() { return S_OK; }

	virtual void EarlyUpdate()
	{
	}

	virtual void Update()
	{
	}

	virtual void LateUpdate()
	{
	}

	virtual void BeforeShutdown()
	{
	}

	virtual void Shutdown()
	{
	}

	virtual void AfterShutdown()
	{
	}

private:
};
