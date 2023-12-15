#pragma once

#include <Engine/Core/Definitions.h>

#include <functional>
#include <Windows.h>

struct SNOWBITE_API FWindowDesc
{
	uint32_t Width = 1280;
	uint32_t Height = 720;
	const char* Title = "Snowbite";
	bool bIsFullscreen = false;
};

struct SNOWBITE_API FWindowState
{
	uint32_t Width = 0;
	uint32_t Height = 0;
	uint32_t ClientWidth = 0;
	uint32_t ClientHeight = 0;
	uint32_t PositionX = 0;
	uint32_t PositionY = 0;
	bool bIsFullscreen = false;
	bool bIsFocused = true;
	bool bIsClosed = false;
};

SNOWBITE_API typedef std::function<void(uint32_t, uint32_t)> FOnResizeCallback;
SNOWBITE_API typedef std::function<void(uint32_t, uint32_t)> FOnClientResizeCallback;
SNOWBITE_API typedef std::function<void(bool)> FOnFocusCallback;
SNOWBITE_API typedef std::function<void(bool)> FOnFullscreenCallback;
SNOWBITE_API typedef std::function<void()> FOnCloseCallback;

class SNOWBITE_API FWindow
{
public:
	FWindow(const FWindowDesc& InDesc);
	~FWindow();

	void Show() const;
	void Hide() const;
	void Maximize() const;
	void Focus() const;
	void Close() const;

	void SetTitle(const char* Title) const;
	void SetSize(uint32_t Width, uint32_t Height) const;
	void SetPosition(uint32_t X, uint32_t Y) const;
	void SetFullscreen(bool bIsFullscreen);

	void SetOnResizeCallback(const FOnResizeCallback& Callback) { OnResizeCallback = Callback; }
	void SetOnClientResizeCallback(const FOnClientResizeCallback& Callback) { OnClientResizeCallback = Callback; }
	void SetOnFocusCallback(const FOnFocusCallback& Callback) { OnFocusCallback = Callback; }
	void SetOnFullscreenCallback(const FOnFullscreenCallback& Callback) { OnFullscreenCallback = Callback; }
	void SetOnCloseCallback(const FOnCloseCallback& Callback) { OnCloseCallback = Callback; }

	[[nodiscard]] FWindowState GetState() const { return State; }

	// Convenience getter functions
	[[nodiscard]] uint32_t GetWidth() const { return State.Width; }
	[[nodiscard]] uint32_t GetHeight() const { return State.Height; }
	[[nodiscard]] uint32_t GetClientWidth() const { return State.ClientWidth; }
	[[nodiscard]] uint32_t GetClientHeight() const { return State.ClientHeight; }
	[[nodiscard]] uint32_t GetPositionX() const { return State.PositionX; }
	[[nodiscard]] uint32_t GetPositionY() const { return State.PositionY; }
	[[nodiscard]] bool IsFullscreen() const { return State.bIsFullscreen; }
	[[nodiscard]] bool IsFocused() const { return State.bIsFocused; }
	[[nodiscard]] bool IsClosed() const { return State.bIsClosed; }

	[[nodiscard]] HWND GetHandle() const { return WindowHandle; }

private:
	void OnMove(uint32_t X, uint32_t Y);
	void OnResize(uint32_t Width, uint32_t Height);
	void OnFocus(bool bIsFocused);
	void OnClose();

private:
	static LRESULT CALLBACK WindowProc(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam);

private:
	inline static WNDCLASSEX WindowClass;

private:
	FWindowState State;
	HWND WindowHandle;

	FOnResizeCallback OnResizeCallback;
	FOnClientResizeCallback OnClientResizeCallback;
	FOnFocusCallback OnFocusCallback;
	FOnFullscreenCallback OnFullscreenCallback;
	FOnCloseCallback OnCloseCallback;
};
