// ReSharper disable CppCStyleCast
#include "Settings.h"

#include <format>

#include "Overlay.h"
#include "resource.h"

#include <shlobj.h>

#pragma comment(lib, "comdlg32.lib")

void Settings::EnsureIniPath()
{
	if (!_iniPath.empty())
	{
		return;
	}

	PWSTR appData = nullptr;

	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &appData)))
	{
		std::wstring dir = std::wstring(appData) + L"\\ClipPing";
		CreateDirectory(dir.c_str(), nullptr);

		_iniPath = dir + L"\\settings.ini";
		CoTaskMemFree(appData);
	}
}

void Settings::Load()
{
	EnsureIniPath();

	if (_iniPath.empty())
	{
		return;
	}

	std::wstring colorBuf(16, L'\0');
	GetPrivateProfileString(L"Overlay", L"Color", L"FF0000", colorBuf.data(), (DWORD)colorBuf.size(), _iniPath.c_str());
	try
	{
		const auto hex = std::stoul(colorBuf, nullptr, 16);
		overlayColor = RGB((hex >> 16) & 0xFF, (hex >> 8) & 0xFF, hex & 0xFF);
	}
	catch (...) {}

	const auto type = GetPrivateProfileInt(L"Overlay", L"Type", 0, _iniPath.c_str());
	
	if (type < OverlayMax)
	{
		overlayType = (OverlayType)type;
	}
}

void Settings::Save()
{
	EnsureIniPath();

	if (_iniPath.empty())
	{
		return;
	}

	const auto colorStr = std::format(L"{:02X}{:02X}{:02X}", GetRValue(overlayColor), GetGValue(overlayColor), GetBValue(overlayColor));
	WritePrivateProfileString(L"Overlay", L"Color", colorStr.c_str(), _iniPath.c_str());

	const auto typeStr = std::to_wstring(overlayType);
	WritePrivateProfileString(L"Overlay", L"Type", typeStr.c_str(), _iniPath.c_str());
}

bool Settings::ShowDialog(HWND parent, HINSTANCE instance, Overlay& overlay)
{
	_dlgColor = overlayColor;
	DlgContext ctx(this, &overlay);
	return DialogBoxParam(instance, MAKEINTRESOURCE(IDD_SETTINGS), parent, DlgProc, (LPARAM)&ctx) == IDOK;
}

INT_PTR CALLBACK Settings::DlgProc(HWND dialog, UINT msg, WPARAM wParam, LPARAM lParam)
{
	auto* ctx = (DlgContext*)GetWindowLongPtr(dialog, DWLP_USER);

	switch (msg)
	{
	case WM_INITDIALOG:
	{
		ctx = (DlgContext*)lParam;
		SetWindowLongPtr(dialog, DWLP_USER, lParam);

		ctx->savedColor = ctx->settings->overlayColor;
		ctx->savedType = ctx->settings->overlayType;

		const auto hCombo = GetDlgItem(dialog, IDC_CMB_OVERLAY);
		SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"Top");
		SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"Border");
		SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"Aura");
		SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"Bottom");
		SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"Left");
		SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"Right");
		SendMessage(hCombo, CB_SETCURSEL, (WPARAM)ctx->settings->overlayType, 0);

		return TRUE;
	}

	case WM_DRAWITEM:
	{
		if (!ctx)
		{
			break;
		}

		const auto dis = (DRAWITEMSTRUCT*)lParam;
		if (dis->CtlID == IDC_COLOR_PREVIEW)
		{
			const auto brush = CreateSolidBrush(ctx->settings->_dlgColor);
			FillRect(dis->hDC, &dis->rcItem, brush);
			DeleteObject(brush);
			return TRUE;
		}
		break;
	}

	case WM_COMMAND:
		if (!ctx)
		{
			break;
		}

		switch (LOWORD(wParam))
		{
		case IDC_BTN_COLOR:
		{
			CHOOSECOLOR cc = {};
			static COLORREF customColors[16] = {};
			cc.lStructSize = sizeof(cc);
			cc.hwndOwner = dialog;
			cc.rgbResult = ctx->settings->_dlgColor;
			cc.lpCustColors = customColors;
			cc.Flags = CC_FULLOPEN | CC_RGBINIT;

			if (ChooseColor(&cc))
			{
				ctx->settings->_dlgColor = cc.rgbResult;
				ctx->settings->overlayColor = cc.rgbResult;
				InvalidateRect(GetDlgItem(dialog, IDC_COLOR_PREVIEW), nullptr, TRUE);
				ctx->overlay->Show();
			}

			return TRUE;
		}

		case IDC_CMB_OVERLAY:
		{
			if (HIWORD(wParam) == CBN_SELCHANGE)
			{
				ctx->settings->overlayType = (OverlayType)SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
				ctx->overlay->Show();
				return TRUE;
			}
			break;
		}

		case IDC_BTN_PREVIEW:
		{
			ctx->overlay->Show();
			return TRUE;
		}

		case IDOK:
		{
			ctx->settings->Save();
			EndDialog(dialog, IDOK);
			return TRUE;
		}

		case IDCANCEL:
			ctx->settings->overlayColor = ctx->savedColor;
			ctx->settings->overlayType = ctx->savedType;
			EndDialog(dialog, IDCANCEL);
			return TRUE;
		}
		break;
	}

	return FALSE;
}
