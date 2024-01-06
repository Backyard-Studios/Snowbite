// ReSharper disable CppClangTidyReadabilityInconsistentDeclarationParameterName
#pragma once


#include <Engine/Core/Definitions.h>

#include "Engine/Core/Types.h"

#include <Windows.h>
#include <string>

class FWindow;

struct SNOWBITE_API FWindowDesc
{
	std::string Title = "Snowbite";
	FUInt2 Size = FUInt2(1280, 720);
	FUInt2 Position = FUInt2(100, 100);
	FUInt2 MinSize = FUInt2(0, 0);
	FUInt2 MaxSize = FUInt2(UINT_MAX, UINT_MAX);
	bool bIsFullscreen = false;
	bool bIsResizable = true;
	bool bIsMaximizable = true;
	bool bIsMinimizable = true;
	bool bShouldRequestExitOnClose = false;
	bool bShouldUnregisterOnClose = false;
	bool bShouldAutoShow = true;
	std::shared_ptr<FWindow> Parent = nullptr;
};

class SNOWBITE_API FWindow
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
	void SetSize(const FUInt2& InSize) const;
	void SetPosition(const FUInt2& InPosition) const;
	void SetTitle(const std::string& InTitle);
	void AppendTitle(const std::string& InTitle);

	void ClearResizeFlag();

	[[nodiscard]] FWindowDesc GetDesc() const { return Desc; }
	[[nodiscard]] HWND GetNativeHandle() const { return NativeHandle; }
	[[nodiscard]] bool IsClosed() const { return bIsClosed; }
	[[nodiscard]] bool WasResized() const { return bWasResized; }

	[[nodiscard]] FUInt2 GetSize() const { return Desc.Size; }
	[[nodiscard]] FUInt2 GetPosition() const { return Desc.Position; }
	[[nodiscard]] bool IsFullscreen() const { return Desc.bIsFullscreen; }
	[[nodiscard]] bool bIsWindowed() const { return !Desc.bIsFullscreen; }
	[[nodiscard]] bool IsValid() const { return NativeHandle != nullptr; }
	[[nodiscard]] bool IsVisible() const { return IsWindowVisible(NativeHandle); }
	[[nodiscard]] bool IsFocused() const { return GetFocus() == NativeHandle; }
	[[nodiscard]] bool IsMinimized() const { return IsIconic(NativeHandle); }
	[[nodiscard]] bool IsMaximized() const { return IsZoomed(NativeHandle); }

private:
	[[nodiscard]] LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);
	void OnResize(uint32_t Width, uint32_t Height);
	void OnClose();
	void OnPositionChanged(uint32_t X, uint32_t Y);

private:
	FWindowDesc Desc;

	HWND NativeHandle;
	bool bIsClosed = false;
	bool bWasResized = false;

	friend class FPlatform;
};
