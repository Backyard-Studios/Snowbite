#pragma once

#include <Engine/Core/Definitions.h>

#include <vector>

#include "Mutex.h"
#include "Window.h"

class SNOWBITE_API FWindowManager
{
public:
	static bool IsRegistered(const std::shared_ptr<FWindow>& Window);
	static bool IsRegistered(HWND WindowHandle);
	static void Register(const std::shared_ptr<FWindow>& Window);
	static void Unregister(const std::shared_ptr<FWindow>& Window);
	static void Unregister(HWND WindowHandle);

	static void HandleWindowMessages();

	[[nodiscard]] static std::shared_ptr<FWindow> GetWindow(HWND WindowHandle);
	[[nodiscard]] static std::vector<std::shared_ptr<FWindow>> GetWindows();

private:
	[[nodiscard]] static HRESULT Initialize();
	static void Shutdown();

private:
	static FMutex Mutex;
	static std::vector<std::shared_ptr<FWindow>> Windows;
};
