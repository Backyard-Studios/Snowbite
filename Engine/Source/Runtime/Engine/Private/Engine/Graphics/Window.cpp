#include "pch.h"

#include <Engine/Graphics/Window.h>

#include <format>
#include <string>

FWindow::FWindow(const FWindowDesc& InDesc)
{
	if (WindowClass.hInstance == nullptr)
	{
		WindowClass.cbSize = sizeof(WNDCLASSEX);
		WindowClass.lpszClassName = "SnowbiteMainWindowClass";
		WindowClass.style = CS_OWNDC;
		WindowClass.lpfnWndProc = WindowProc;
		WindowClass.cbClsExtra = 0;
		WindowClass.cbWndExtra = 0;
		WindowClass.hInstance = GetModuleHandle(nullptr);
		WindowClass.hIcon = nullptr;
		WindowClass.hCursor = nullptr;
		WindowClass.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
		WindowClass.lpszMenuName = nullptr;
		WindowClass.hIconSm = nullptr;
		RegisterClassEx(&WindowClass);
	}
	WindowHandle = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW, WindowClass.lpszClassName, InDesc.Title,
	                              WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, InDesc.Width,
	                              InDesc.Height,
	                              nullptr,
	                              nullptr, WindowClass.hInstance, this);
	SB_ASSERT_CRITICAL(WindowHandle != nullptr, E_FAIL, "Failed to create window!");
}

FWindow::~FWindow()
{
	if (IsWindow(WindowHandle))
	{
		if (!DestroyWindow(WindowHandle))
		{
			const DWORD Error = GetLastError();
			LPSTR MessageBuffer = nullptr;
			const size_t Size = FormatMessageA(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr, Error, MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&MessageBuffer), 0,
				nullptr);
			std::string Message(MessageBuffer, Size);
			LocalFree(MessageBuffer);
			SB_ASSERT(false, Error,
			          std::format("Failed to destroy window! {}", Message.replace(Message.find('\n'), 1, "")).c_str());
		}
	}
	UnregisterClass(WindowClass.lpszClassName, WindowClass.hInstance);
}

void FWindow::Show() const
{
	ShowWindow(WindowHandle, SW_SHOW);
}

void FWindow::Hide() const
{
	ShowWindow(WindowHandle, SW_HIDE);
}

void FWindow::Maximize() const
{
	ShowWindow(WindowHandle, SW_MAXIMIZE);
}

void FWindow::Focus() const
{
	SetFocus(WindowHandle);
}

void FWindow::Close() const
{
	CloseWindow(WindowHandle);
}

void FWindow::SetTitle(const char* Title) const
{
	SetWindowText(WindowHandle, Title);
}

void FWindow::SetSize(const uint32_t Width, const uint32_t Height) const
{
	SetWindowPos(WindowHandle, HWND_TOP, 0, 0, Width, Height, SWP_NOMOVE);
}

void FWindow::SetPosition(const uint32_t X, const uint32_t Y) const
{
	SetWindowPos(WindowHandle, HWND_TOP, X, Y, 0, 0, SWP_NOSIZE);
}

void FWindow::SetFullscreen(const bool bIsFullscreen)
{
	State.bIsFullscreen = bIsFullscreen;
	if (OnFullscreenCallback)
		OnFullscreenCallback(bIsFullscreen);
}

void FWindow::OnMove(const uint32_t X, const uint32_t Y)
{
	State.PositionX = X;
	State.PositionY = Y;
}

void FWindow::OnResize(const uint32_t Width, const uint32_t Height)
{
	State.Width = Width;
	State.Height = Height;
	if (OnResizeCallback)
		OnResizeCallback(Width, Height);
	RECT ClientRect = {};
	GetClientRect(WindowHandle, &ClientRect);
	State.ClientWidth = ClientRect.right - ClientRect.left;
	State.ClientHeight = ClientRect.bottom - ClientRect.top;
	if (OnClientResizeCallback)
		OnClientResizeCallback(State.ClientWidth, State.ClientHeight);
}

void FWindow::OnFocus(const bool bIsFocused)
{
	State.bIsFocused = bIsFocused;
	if (OnFocusCallback)
		OnFocusCallback(bIsFocused);
}

void FWindow::OnClose()
{
	State.bIsClosed = true;
	if (OnCloseCallback)
		OnCloseCallback();
}

LRESULT FWindow::WindowProc(const HWND WindowHandle, const UINT Message, const WPARAM WParam, const LPARAM LParam)
{
	FWindow* Window = reinterpret_cast<FWindow*>(GetWindowLongPtr(WindowHandle, GWLP_USERDATA));
	switch (Message)
	{
	case WM_CREATE:
		Window = static_cast<FWindow*>(reinterpret_cast<CREATESTRUCT*>(LParam)->lpCreateParams);
		SetWindowLongPtr(WindowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(Window));
		break;
	case WM_ACTIVATE:
		Window->OnFocus(LOWORD(WParam) != WA_INACTIVE);
		break;
	case WM_SIZE:
		{
			const uint32_t Width = LOWORD(LParam);
			const uint32_t Height = HIWORD(LParam);
			if ((Width != Window->GetState().Width || Height != Window->GetState().Height) && (Width != 0 && Height !=
				0))
				Window->OnResize(Width, Height);
		}
		break;
	case WM_CLOSE:
		Window->OnClose();
		break;
	case WM_MOVE:
		{
			const uint32_t X = LOWORD(LParam);
			const uint32_t Y = HIWORD(LParam);
			if (X != Window->GetState().PositionX || Y != Window->GetState().PositionY)
				Window->OnMove(X, Y);
		}
	default:
		break;
	}
	return DefWindowProc(WindowHandle, Message, WParam, LParam);
}
