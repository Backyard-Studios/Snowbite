#pragma once

#include <vector>

#include "Mutex.h"
#include "Window.h"
#include "Engine/Core/EngineService.h"

class FWindowManager
{
public:
	static bool IsRegistered(const std::shared_ptr<FWindow>& Window);
	static bool IsRegistered(HWND WindowHandle);
	static void Register(const std::shared_ptr<FWindow>& Window);
	static void Unregister(const std::shared_ptr<FWindow>& Window);
	static void Unregister(HWND WindowHandle);

	static std::shared_ptr<FWindow> GetWindow(HWND WindowHandle);

private:
	static FMutex Mutex;
	static std::vector<std::shared_ptr<FWindow>> Windows;

	friend class FWindowManagerService;
};

class FWindowManagerService : public IEngineService
{
public:
	HRESULT Initialize() override;
	void EarlyUpdate() override;
	void Shutdown() override;

private:
};
