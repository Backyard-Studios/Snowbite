// ReSharper disable CppClangTidyReadabilityInconsistentDeclarationParameterName
#pragma once

#include "Engine/Core/Types.h"

#include <Windows.h>

class FWindow;

struct FWindowDesc
{
	std::string Title = "Snowbite";
	FFloat2 Size = FFloat2(1280, 720);
	FFloat2 Position = FFloat2(100, 100);
	FFloat2 MinSize = FFloat2(0, 0);
	FFloat2 MaxSize = FFloat2(FLT_MAX, FLT_MAX);
	bool bIsFullscreen = false;
	bool bIsResizable = true;
	bool bIsMaximizable = true;
	bool bIsMinimizable = true;
	bool bShouldRequestExitOnClose = false;
	bool bShouldUnregisterOnClose = false;
	bool bShouldAutoShow = true;
	std::shared_ptr<FWindow> Parent = nullptr;
};

class FWindow
{
public:
	FWindow(const FWindowDesc& InDesc);
	~FWindow();

	void HandleMessages() const;

	void Show() const;
	void Hide() const;
	void Maximize() const;
	void Minimize() const;
	void Restore() const;
	void Focus() const;
	void Close() const;
	void Flash() const;

	void SetFullscreen(bool bShouldFullscreen);
	void SetSize(const FFloat2& InSize) const;
	void SetPosition(const FFloat2& InPosition) const;
	void SetTitle(const std::string& InTitle) const;

	[[nodiscard]] FWindowDesc GetDesc() const { return Desc; }
	[[nodiscard]] HWND GetNativeHandle() const { return NativeHandle; }
	[[nodiscard]] bool IsClosed() const { return bIsClosed; }

	[[nodiscard]] FFloat2 GetSize() const { return Desc.Size; }
	[[nodiscard]] FFloat2 GetPosition() const { return Desc.Position; }
	[[nodiscard]] bool IsFullscreen() const { return Desc.bIsFullscreen; }
	[[nodiscard]] bool bIsWindowed() const { return !Desc.bIsFullscreen; }
	[[nodiscard]] bool IsValid() const { return NativeHandle != nullptr; }
	[[nodiscard]] bool IsVisible() const { return IsWindowVisible(NativeHandle); }
	[[nodiscard]] bool IsFocused() const { return GetFocus() == NativeHandle; }
	[[nodiscard]] bool IsMinimized() const { return IsIconic(NativeHandle); }
	[[nodiscard]] bool IsMaximized() const { return IsZoomed(NativeHandle); }

private:
	LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

private:
	FWindowDesc Desc;

	HWND NativeHandle;
	bool bIsClosed = false;

	friend class FPlatform;
};
