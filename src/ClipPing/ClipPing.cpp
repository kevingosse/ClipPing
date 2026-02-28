/*
 * ClipPing - Displays a visual notification in the active window when the clipboard is updated.
 */

// ReSharper disable CppCStyleCast
#include <cstdint>

#define NOMINMAX

#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>
#include <gdiplus.h>

#include "resource.h"
#include "Overlay.h"
#include "Settings.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "shcore.lib")
#pragma comment(lib, "shell32.lib")

#define WM_TRAYICON     (WM_APP + 1)

struct AppState
{
	Settings settings;
	Overlay overlay;
	NOTIFYICONDATA nid = {};
	HINSTANCE hInstance = nullptr;

	explicit AppState(HINSTANCE h) : overlay(settings), hInstance(h) {}

	static INT_PTR CALLBACK AboutDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_NOTIFY:
			{
				const auto nmhdr = (NMHDR*)lParam;
				if (nmhdr->code == NM_CLICK || nmhdr->code == NM_RETURN)
				{
					const auto link = (NMLINK*)lParam;
					ShellExecute(nullptr, L"open", link->item.szUrl, nullptr, nullptr, SW_SHOWNORMAL);
					return TRUE;
				}
			}
			break;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hwnd, LOWORD(wParam));
				return TRUE;
			}
			break;
		}

		return FALSE;
	}

	static void ShowTrayMenu(HWND hwnd)
	{
		const auto menu = CreatePopupMenu();
		AppendMenu(menu, MF_STRING, IDM_SETTINGS, L"Settings...");
		AppendMenu(menu, MF_STRING, IDM_ABOUT, L"About...");
		AppendMenu(menu, MF_SEPARATOR, 0, nullptr);
		AppendMenu(menu, MF_STRING, IDM_EXIT, L"Exit");

		SetForegroundWindow(hwnd);

		POINT pt;
		GetCursorPos(&pt);
		TrackPopupMenu(menu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, nullptr);
		
		// For notification icon context menus, the Win32 guidance is to call PostMessage(hwnd, WM_NULL, 0, 0) 
		// to ensure the menu reliably dismisses when the user clicks elsewhere.
		PostMessage(hwnd, WM_NULL, 0, 0);
		DestroyMenu(menu);
	}

	void InitTrayIcon(HWND hwnd)
	{
		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = hwnd;
		nid.uID = 1;
		nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		nid.uCallbackMessage = WM_TRAYICON;
		wcscpy_s(nid.szTip, L"ClipPing");

		nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLIPPING));

		Shell_NotifyIcon(NIM_ADD, &nid);
	}

	void RemoveTrayIcon()
	{
		Shell_NotifyIcon(NIM_DELETE, &nid);

		if (nid.hIcon)
		{
			DestroyIcon(nid.hIcon);
		}
	}

	static LRESULT CALLBACK ListenerWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (msg == WM_NCCREATE)
		{
			const auto cs = (CREATESTRUCT*)lParam;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}

		auto* app = (AppState*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		if (!app)
		{
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}

		switch (msg)
		{
		case WM_CLIPBOARDUPDATE:
			app->overlay.Show();
			return 0;

		case WM_TRAYICON:
			if (LOWORD(lParam) == WM_RBUTTONUP)
			{
				ShowTrayMenu(hwnd);
			}
			else if (LOWORD(lParam) == WM_LBUTTONDBLCLK)
			{
				app->settings.ShowDialog(hwnd, app->hInstance, app->overlay);
			}

			return 0;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDM_EXIT)
			{
				DestroyWindow(hwnd);
			}
			else if (LOWORD(wParam) == IDM_SETTINGS)
			{
				app->settings.ShowDialog(hwnd, app->hInstance, app->overlay);
			}
			else if (LOWORD(wParam) == IDM_ABOUT)
			{
				DialogBox(app->hInstance, MAKEINTRESOURCE(IDD_ABOUT), hwnd, AboutDlgProc);
			}

			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		}

		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int)
{
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	AppState app(hInstance);
	app.settings.Load();

	ULONG_PTR gdiplusToken = 0;
	Gdiplus::GdiplusStartupInput gdiplusStartup;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartup, nullptr);

	WNDCLASS wc = {};

	wc.lpfnWndProc = AppState::ListenerWndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = L"ClipPingListener";
	RegisterClass(&wc);

	wc.lpfnWndProc = Overlay::WndProc;
	wc.lpszClassName = L"ClipPingOverlay";
	wc.hbrBackground = nullptr;
	RegisterClass(&wc);

	const auto hwndListener = CreateWindowEx(
		WS_EX_TOOLWINDOW,
		L"ClipPingListener", L"",
		WS_POPUP,
		0, 0, 0, 0,
		nullptr, nullptr, hInstance, &app);

	if (!hwndListener)
	{
		return 1;
	}

	AddClipboardFormatListener(hwndListener);
	app.InitTrayIcon(hwndListener);

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	app.RemoveTrayIcon();
	RemoveClipboardFormatListener(hwndListener);

	Gdiplus::GdiplusShutdown(gdiplusToken);

	return (int)msg.wParam;
}
