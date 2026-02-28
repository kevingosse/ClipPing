// ReSharper disable CppCStyleCast
#include <algorithm>
#include <cstdint>

#define NOMINMAX

#include <windows.h>
#include <gdiplus.h>
#include <dwmapi.h>

#include "Overlay.h"
#include "Settings.h"

using namespace Gdiplus;

constexpr double Overlay::EaseOutCubic(const double t)
{
	const double u = 1.0 - t;
	return 1.0 - u * u * u;
}

constexpr double Overlay::EaseInCubic(const double t)
{
	return t * t * t;
}

RECT Overlay::GetForegroundWindowRect()
{
	RECT rect = {};
	const auto hwnd = GetForegroundWindow();

	if (!hwnd)
	{
		return rect;
	}

	const auto hr = DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(rect));

	if (FAILED(hr))
	{
		GetWindowRect(hwnd, &rect);
	}

	return rect;
}

Overlay::Overlay(const Settings& settings)
	: _settings(settings)
{
}

Overlay::~Overlay()
{
	if (_bitmap)
	{
		DeleteObject(_bitmap);
	}
}

void Overlay::UpdateAlpha(int32_t alpha) const
{
	if (!_hwnd || !_bitmap)
	{
		return;
	}

	alpha = std::clamp(alpha, 0, 255);

	const auto deviceContext = GetDC(nullptr);

	if (!deviceContext)
	{
		return;
	}

	const auto memoryDeviceContext = CreateCompatibleDC(deviceContext);

	if (!memoryDeviceContext)
	{
		ReleaseDC(nullptr, deviceContext);
		return;
	}

	const auto old = (HBITMAP)SelectObject(memoryDeviceContext, _bitmap);

	POINT ptSrc = { 0, 0 };
	SIZE sizeWnd = { _bitmapWidth, _bitmapHeight };

	BLENDFUNCTION blend = {};
	blend.BlendOp = AC_SRC_OVER;
	blend.SourceConstantAlpha = (BYTE)alpha;
	blend.AlphaFormat = AC_SRC_ALPHA;

	RECT rc;
	GetWindowRect(_hwnd, &rc);
	POINT ptDst = { rc.left, rc.top };

	UpdateLayeredWindow(_hwnd, deviceContext, &ptDst, &sizeWnd, memoryDeviceContext, &ptSrc, 0, &blend, ULW_ALPHA);

	SelectObject(memoryDeviceContext, old);
	DeleteDC(memoryDeviceContext);
	ReleaseDC(nullptr, deviceContext);
}

void Overlay::CreateBitmap(const int32_t width, const int32_t height)
{
	if (_bitmap)
	{
		DeleteObject(_bitmap);
		_bitmap = nullptr;
	}

	_bitmapWidth = width;
	_bitmapHeight = height;

	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = -height; // Top-down
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	void* bits = nullptr;
	const auto hdcScreen = GetDC(nullptr);
	_bitmap = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
	ReleaseDC(nullptr, hdcScreen);

	if (!_bitmap || !bits)
	{
		return;
	}

	const int32_t stride = width * 4;
	Bitmap gpBitmap(width, height, stride, PixelFormat32bppPARGB, (BYTE*)bits);

	Graphics graphics(&gpBitmap);
	graphics.Clear(Color(0, 0, 0, 0));

	const BYTE r = GetRValue(_settings.overlayColor);
	const BYTE g = GetGValue(_settings.overlayColor);
	const BYTE b = GetBValue(_settings.overlayColor);

	switch (_settings.overlayType)
	{
	case OverlayBorder:
		CreateBorderBitmap(graphics, width, height, r, g, b);
		break;
	case OverlayAura:
		CreateAuraBitmap(graphics, width, height, r, g, b);
		break;
	case OverlayBottom:
		CreateBottomBitmap(graphics, width, height, r, g, b);
		break;
	case OverlayLeft:
		CreateLeftBitmap(graphics, width, height, r, g, b);
		break;
	case OverlayRight:
		CreateRightBitmap(graphics, width, height, r, g, b);
		break;
	case OverlayTop:
		CreateTopBitmap(graphics, width, height, r, g, b);
		break;
	}
}

void Overlay::CreateTopBitmap(Graphics& graphics, const int32_t width, const int32_t height, const BYTE r, const BYTE g, const BYTE b)
{
	int32_t gradientHeight = height * GradientHeightPct / 100;
	gradientHeight = std::max(gradientHeight, 2);

	const LinearGradientBrush brush(
		Point(0, 0),
		Point(0, gradientHeight),
		Color(0x50, r, g, b),
		Color(0x00, r, g, b));

	graphics.FillRectangle(&brush, 0, 0, width, gradientHeight);
}

void Overlay::CreateBottomBitmap(Graphics& graphics, const int32_t width, const int32_t height, const BYTE r, const BYTE g, const BYTE b)
{
	int32_t gradientHeight = height * GradientHeightPct / 100;
	gradientHeight = std::max(gradientHeight, 2);

	LinearGradientBrush brush(
		Point(0, height - gradientHeight),
		Point(0, height),
		Color(0x00, r, g, b),
		Color(0x50, r, g, b));
	brush.SetWrapMode(WrapModeTileFlipXY);

	graphics.FillRectangle(&brush, 0, height - gradientHeight, width, gradientHeight);
}

void Overlay::CreateLeftBitmap(Graphics& graphics, const int32_t width, const int32_t height, const BYTE r, const BYTE g, const BYTE b)
{
	int32_t gradientWidth = width * GradientHeightPct / 100;
	gradientWidth = std::max(gradientWidth, 2);

	const LinearGradientBrush brush(
		Point(0, 0),
		Point(gradientWidth, 0),
		Color(0x50, r, g, b),
		Color(0x00, r, g, b));

	graphics.FillRectangle(&brush, 0, 0, gradientWidth, height);
}

void Overlay::CreateRightBitmap(Graphics& graphics, const int32_t width, const int32_t height, const BYTE r, const BYTE g, const BYTE b)
{
	int32_t gradientWidth = width * GradientHeightPct / 100;
	gradientWidth = std::max(gradientWidth, 2);

	const LinearGradientBrush brush(
		Point(width - gradientWidth, 0),
		Point(width, 0),
		Color(0x00, r, g, b),
		Color(0x50, r, g, b));

	graphics.FillRectangle(&brush, width - gradientWidth, 0, gradientWidth, height);
}

void Overlay::CreateBorderBitmap(Graphics& graphics, const int32_t width, const int32_t height, const BYTE r, const BYTE g, const BYTE b)
{
	const SolidBrush brush(Color(0x80, r, g, b));
	const int t = BorderThickness;

	// Top strip
	graphics.FillRectangle(&brush, 0, 0, width, t);
	// Bottom strip
	graphics.FillRectangle(&brush, 0, height - t, width, t);
	// Left strip
	graphics.FillRectangle(&brush, 0, t, t, height - 2 * t);
	// Right strip
	graphics.FillRectangle(&brush, width - t, t, t, height - 2 * t);
}

void Overlay::CreateAuraBitmap(Graphics& graphics, const int32_t width, const int32_t height, const BYTE r, const BYTE g, const BYTE b)
{
	int32_t depth = height * AuraDepthPct / 100;
	depth = std::max(depth, 2);

	// Top
	{
		const LinearGradientBrush brush(
			Point(0, 0),
			Point(0, depth),
			Color(0x50, r, g, b),
			Color(0x00, r, g, b));
		graphics.FillRectangle(&brush, 0, 0, width, depth);
	}

	// Bottom
	{
		const LinearGradientBrush brush(
			Point(0, height - depth),
			Point(0, height),
			Color(0x00, r, g, b),
			Color(0x50, r, g, b));
		graphics.FillRectangle(&brush, 0, height - depth, width, depth);
	}

	// Left
	{
		const LinearGradientBrush brush(
			Point(0, 0),
			Point(depth, 0),
			Color(0x50, r, g, b),
			Color(0x00, r, g, b));
		graphics.FillRectangle(&brush, 0, 0, depth, height);
	}

	// Right
	{
		const LinearGradientBrush brush(
			Point(width - depth, 0),
			Point(width, 0),
			Color(0x00, r, g, b),
			Color(0x50, r, g, b));
		graphics.FillRectangle(&brush, width - depth, 0, depth, height);
	}
}

void Overlay::OnTimer()
{
	const auto now = GetTickCount();
	const auto elapsed = now - _animStart;

	if (_phase == AnimFadeIn)
	{
		if (elapsed >= FadeInMs)
		{
			UpdateAlpha(255);
			_phase = AnimFadeOut;
			_animStart = now;
		}
		else
		{
			const double t = (double)elapsed / FadeInMs;
			const auto alpha = (int32_t)(EaseOutCubic(t) * 255.0);
			UpdateAlpha(alpha);
		}
	}
	else if (_phase == AnimFadeOut)
	{
		if (elapsed >= FadeOutMs)
		{
			KillTimer(_hwnd, 1);
			UpdateAlpha(0);

			if (_bitmap)
			{
				DeleteObject(_bitmap);
				_bitmap = nullptr;
				_bitmapWidth = 0;
				_bitmapHeight = 0;
			}

			_phase = AnimNone;
		}
		else
		{
			const double t = (double)elapsed / FadeOutMs;
			const auto alpha = (int32_t)((1.0 - EaseInCubic(t)) * 255.0);
			UpdateAlpha(alpha);
		}
	}
}

LRESULT CALLBACK Overlay::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_NCCREATE)
	{
		const auto cs = (CREATESTRUCT*)lParam;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	auto* self = (Overlay*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if (msg == WM_TIMER && wParam == 1 && self)
	{
		self->OnTimer();
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void Overlay::Show()
{
	if (_phase != AnimNone)
	{
		return;
	}

	const auto foregroundRect = GetForegroundWindowRect();
	const int width = foregroundRect.right - foregroundRect.left;

	// Shrink by 1px, otherwise Windows detects this as a fullscreen window and automatically enables Focus Assist
	const int height = foregroundRect.bottom - foregroundRect.top - 1;

	if (width <= 0 || height <= 0)
	{
		return;
	}

	if (!_hwnd)
	{
		_hwnd = CreateWindowEx(
			WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
			L"ClipPingOverlay", L"",
			WS_POPUP | WS_VISIBLE,
			foregroundRect.left, foregroundRect.top, width, height,
			nullptr, nullptr, GetModuleHandle(nullptr), this);
	}
	else
	{
		SetWindowPos(_hwnd, HWND_TOPMOST, foregroundRect.left, foregroundRect.top, width, height, SWP_NOACTIVATE);
	}

	if (!_hwnd)
	{
		return;
	}

	CreateBitmap(width, height);

	_phase = AnimFadeIn;
	_animStart = GetTickCount();
	UpdateAlpha(0);
	SetTimer(_hwnd, 1, TimerInterval, nullptr);
}
