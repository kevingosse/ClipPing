#pragma once

#include <cstdint>
#include <windows.h>
#include <gdiplus.h>

class Settings;

class Overlay
{
public:
	explicit Overlay(const Settings& settings);
	~Overlay();

	Overlay(const Overlay&) = delete;
	Overlay& operator=(const Overlay&) = delete;

	void Show();

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	void CreateBitmap(int32_t width, int32_t height);
	void UpdateAlpha(int32_t alpha) const;
	void OnTimer();

	static void CreateTopBitmap(Gdiplus::Graphics& graphics, int32_t width, int32_t height, BYTE r, BYTE g, BYTE b);
	static void CreateBottomBitmap(Gdiplus::Graphics& graphics, int32_t width, int32_t height, BYTE r, BYTE g, BYTE b);
	static void CreateLeftBitmap(Gdiplus::Graphics& graphics, int32_t width, int32_t height, BYTE r, BYTE g, BYTE b);
	static void CreateRightBitmap(Gdiplus::Graphics& graphics, int32_t width, int32_t height, BYTE r, BYTE g, BYTE b);
	static void CreateBorderBitmap(Gdiplus::Graphics& graphics, int32_t width, int32_t height, BYTE r, BYTE g, BYTE b);
	static void CreateAuraBitmap(Gdiplus::Graphics& graphics, int32_t width, int32_t height, BYTE r, BYTE g, BYTE b);

	static constexpr double EaseOutCubic(double t);
	static constexpr double EaseInCubic(double t);
	static RECT GetForegroundWindowRect();

	enum AnimPhase : uint8_t { AnimNone, AnimFadeIn, AnimFadeOut };

	static constexpr int FadeInMs = 100;
	static constexpr int FadeOutMs = 300;
	static constexpr int TimerInterval = 16;
	static constexpr int GradientHeightPct = 10;
	static constexpr int AuraDepthPct = 5;
	static constexpr int BorderThickness = 8;

	const Settings& _settings;
	HWND _hwnd = nullptr;
	HBITMAP _bitmap = nullptr;
	int32_t _bitmapWidth = 0;
	int32_t _bitmapHeight = 0;
	AnimPhase _phase = AnimNone;
	DWORD _animStart = 0;
};
