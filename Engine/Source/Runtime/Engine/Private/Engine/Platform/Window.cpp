#include "pch.h"

#include <Engine/Platform/Window.h>

FWindow::FWindow(const FWindowDesc& InDesc)
	: Desc(InDesc)
{
	uint32_t Style = WS_OVERLAPPEDWINDOW;
	constexpr uint32_t ExStyle = WS_EX_APPWINDOW;
	if (Desc.bIsMaximizable)
		Style |= WS_MAXIMIZEBOX;
	if (Desc.bIsMinimizable)
		Style |= WS_MINIMIZEBOX;
	NativeHandle = CreateWindowEx(ExStyle, FPlatform::GetWindowClass().lpszClassName, Desc.Title.c_str(), Style,
	                              static_cast<int>(Desc.Position.X), static_cast<int>(Desc.Position.Y),
	                              static_cast<int>(Desc.Size.X), static_cast<int>(Desc.Size.Y),
	                              Desc.Parent != nullptr ? Desc.Parent->GetNativeHandle() : nullptr, nullptr,
	                              FPlatform::GetWindowClass().hInstance, this);
	if (NativeHandle == nullptr)
		FPlatform::Fatal();
	if (Desc.bShouldAutoShow)
		Show();
	SB_LOG_INFO("Created new window: {} ({}x{})", Desc.Title, Desc.Size.X, Desc.Size.Y);
}

FWindow::~FWindow()
{
	if (IsWindow(NativeHandle))
		DestroyWindow(NativeHandle);
}

void FWindow::HandleMessages() const
{
	MSG Message;
	while (PeekMessage(&Message, NativeHandle, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
}

void FWindow::Show() const
{
	ShowWindow(NativeHandle, SW_SHOW);
}

void FWindow::Hide() const
{
	ShowWindow(NativeHandle, SW_HIDE);
}

void FWindow::Maximize() const
{
	ShowWindow(NativeHandle, SW_MAXIMIZE);
}

void FWindow::Minimize() const
{
	ShowWindow(NativeHandle, SW_MINIMIZE);
}

void FWindow::Restore() const
{
	ShowWindow(NativeHandle, SW_RESTORE);
}

void FWindow::Focus() const
{
	SetFocus(NativeHandle);
}

void FWindow::Close() const
{
	CloseWindow(NativeHandle);
}

void FWindow::Flash() const
{
	FlashWindow(NativeHandle, true);
}

void FWindow::SetFullscreen(const bool bShouldFullscreen)
{
	Desc.bIsFullscreen = bShouldFullscreen;
}

void FWindow::SetSize(const FUInt2& InSize) const
{
	SetWindowPos(NativeHandle, nullptr, 0, 0, InSize.X, InSize.Y, SWP_NOMOVE | SWP_NOZORDER);
}

void FWindow::SetPosition(const FUInt2& InPosition) const
{
	SetWindowPos(NativeHandle, nullptr, InPosition.X, InPosition.Y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void FWindow::SetTitle(const std::string& InTitle) const
{
	SetWindowText(NativeHandle, InTitle.c_str());
}

LRESULT FWindow::WndProc(const UINT Message, const WPARAM WParam, const LPARAM LParam)
{
	switch (Message)
	{
	case WM_SIZE:
		{
			const uint32_t Width = LOWORD(LParam);
			const uint32_t Height = HIWORD(LParam);
			if ((Width != Desc.Size.X || Height != Desc.Size.Y) && (Width != 0 && Height != 0))
				OnResize(Width, Height);
		}
		break;
	case WM_MOVE:
		{
			const uint32_t X = LOWORD(LParam);
			const uint32_t Y = HIWORD(LParam);
			if (X != Desc.Position.X || Y != Desc.Position.Y)
				OnPositionChanged(X, Y);
		}
		break;
	case WM_CLOSE:
		OnClose();
		break;
	default:
		break;
	}
	return DefWindowProc(NativeHandle, Message, WParam, LParam);
}

void FWindow::OnResize(const uint32_t Width, const uint32_t Height)
{
	Desc.Size = FUInt2(Width, Height);
}

void FWindow::OnClose()
{
	bIsClosed = true;
	if (Desc.bShouldUnregisterOnClose)
		FWindowManager::Unregister(NativeHandle);
	if (Desc.bShouldRequestExitOnClose)
		FEngine::RequestExit();
}

void FWindow::OnPositionChanged(const uint32_t X, const uint32_t Y)
{
	Desc.Position = FUInt2(X, Y);
}
