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

void FWindowManager::HandleWindowMessages()
{
	for (const std::shared_ptr<FWindow> Window : Windows)
		Window->HandleMessages();
}

std::shared_ptr<FWindow> FWindowManager::GetWindow(HWND WindowHandle)
{
	return *std::ranges::find_if(Windows, [WindowHandle](const std::shared_ptr<FWindow>& Window)
	{
		return Window->GetNativeHandle() == WindowHandle;
	});
}

std::vector<std::shared_ptr<FWindow>> FWindowManager::GetWindows()
{
	return Windows;
}

HRESULT FWindowManager::Initialize()
{
	return S_OK;
}

void FWindowManager::Shutdown()
{
	for (std::shared_ptr<FWindow>& Window : Windows)
		Window.reset();
	Windows.clear();
}
