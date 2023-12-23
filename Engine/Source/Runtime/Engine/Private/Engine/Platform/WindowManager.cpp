#include "pch.h"

#include <Engine/Platform/WindowManager.h>

FMutex FWindowManager::Mutex;
std::vector<std::shared_ptr<FWindow>> FWindowManager::Windows;

bool FWindowManager::IsRegistered(const std::shared_ptr<FWindow>& Window)
{
	return std::ranges::find_if(Windows, [Window](const std::shared_ptr<FWindow>& RegisteredWindow)
	{
		return RegisteredWindow->GetNativeHandle() == Window->GetNativeHandle();
	}) != Windows.end();
}

bool FWindowManager::IsRegistered(HWND WindowHandle)
{
	return std::ranges::find_if(Windows, [WindowHandle](const std::shared_ptr<FWindow>& RegisteredWindow)
	{
		return RegisteredWindow->GetNativeHandle() == WindowHandle;
	}) != Windows.end();
}

void FWindowManager::Register(const std::shared_ptr<FWindow>& Window)
{
	if (IsRegistered(Window))
		return;
	FMutexGuard Guard(Mutex);
	Windows.push_back(Window);
}

void FWindowManager::Unregister(const std::shared_ptr<FWindow>& Window)
{
	if (!IsRegistered(Window))
		return;
	FMutexGuard Guard(Mutex);
	Windows.erase(std::ranges::find_if(Windows, [Window](const std::shared_ptr<FWindow>& RegisteredWindow)
	{
		return RegisteredWindow->GetNativeHandle() == Window->GetNativeHandle();
	}));
}

void FWindowManager::Unregister(HWND WindowHandle)
{
	if (!IsRegistered(WindowHandle))
		return;
	FMutexGuard Guard(Mutex);
	Windows.erase(std::ranges::find_if(Windows, [WindowHandle](const std::shared_ptr<FWindow>& RegisteredWindow)
	{
		return RegisteredWindow->GetNativeHandle() == WindowHandle;
	}));
}

std::shared_ptr<FWindow> FWindowManager::GetWindow(HWND WindowHandle)
{
	return *std::ranges::find_if(Windows, [WindowHandle](const std::shared_ptr<FWindow>& Window)
	{
		return Window->GetNativeHandle() == WindowHandle;
	});
}

HRESULT FWindowManagerService::Initialize()
{
	return IEngineService::Initialize();
}

void FWindowManagerService::EarlyUpdate()
{
	for (const std::shared_ptr<FWindow> Window : FWindowManager::Windows)
		Window->HandleMessages();
	IEngineService::EarlyUpdate();
}

void FWindowManagerService::Shutdown()
{
	for (std::shared_ptr<FWindow> Window : FWindowManager::Windows)
	{
		Window->Close();
		Window.reset();
	}
	FWindowManager::Windows.clear();
	IEngineService::Shutdown();
}
