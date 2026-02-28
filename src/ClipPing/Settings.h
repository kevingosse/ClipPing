#pragma once

#include <windows.h>
#include <cstdint>
#include <string>

class Overlay;

enum OverlayType : int32_t
{
	OverlayTop = 0,
	OverlayBorder = 1,
	OverlayAura = 2,
	OverlayBottom = 3,
	OverlayLeft = 4,
	OverlayRight = 5,
	OverlayMax
};

class Settings
{
public:
	void Load();
	void Save();
	bool ShowDialog(HWND parent, HINSTANCE instance, Overlay& overlay);

	static bool GetAutoStart();
	static void SetAutoStart(bool enable);

	COLORREF overlayColor = RGB(255, 0, 0);
	OverlayType overlayType = OverlayTop;

private:
	struct DlgContext
	{
		Settings* settings;
		Overlay* overlay;
		COLORREF savedColor = 0;
		OverlayType savedType = OverlayTop;

		DlgContext(Settings* s, Overlay* o) : settings(s), overlay(o) {}
	};

	static INT_PTR CALLBACK DlgProc(HWND dialog, UINT msg, WPARAM wParam, LPARAM lParam);
	void EnsureIniPath();

	COLORREF _dlgColor = 0;
	std::wstring _iniPath;
};
