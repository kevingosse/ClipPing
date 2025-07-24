using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;
using System.Runtime.InteropServices;

namespace Kookiz.ClipPing;

public partial class App : Application
{
    /// <summary>
    /// A window used only for the message loop
    /// </summary>
    private Window? _dummyWindow;

    public override void Initialize()
    {
        AvaloniaXamlLoader.Load(this);
    }

    public override void OnFrameworkInitializationCompleted()
    {
        var applicationLifetime = ApplicationLifetime as IClassicDesktopStyleApplicationLifetime;

        if (applicationLifetime == null)
        {
            return;
        }

        applicationLifetime.ShutdownMode = ShutdownMode.OnExplicitShutdown;

        _dummyWindow = new Window();
        Win32Properties.AddWndProcHookCallback(_dummyWindow, new(WndProcHookCallback));

        var hwnd = _dummyWindow.TryGetPlatformHandle()?.Handle;

        if (hwnd == null)
        {
            // TODO: how to detect the editor
            return;
        }

        var result = NativeMethods.AddClipboardFormatListener(hwnd.Value);

        if (!result)
        {
            // TODO: Need a message box here
            applicationLifetime.Shutdown();
        }

        base.OnFrameworkInitializationCompleted();
    }

    private IOverlay LoadOverlay()
    {
        // TODO: Add a way to pick what overlay to use
        var overlay = new TopOverlay
        {
            ShowInTaskbar = false,
            ShowActivated = false,
            IsHitTestVisible = false,
            SystemDecorations = SystemDecorations.None,
            Topmost = true
        };

        Win32Properties.AddWindowStylesCallback(
            overlay,
            (style, exStyle) => (style, exStyle | NativeMethods.WS_EX_TRANSPARENT | NativeMethods.WS_EX_LAYERED));

        return overlay;
    }

    private IntPtr WndProcHookCallback(IntPtr hWnd, uint msg, IntPtr wParam, IntPtr lParam, ref bool handled)
    {
        if (msg == 0x031D /* WM_CLIPBOARDUPDATE */)
        {
            _ = ShowOverlay();
        }

        return IntPtr.Zero;
    }

    private async Task ShowOverlay()
    {
        var overlay = LoadOverlay();

        var hwnd = NativeMethods.GetForegroundWindow();

        if (hwnd == IntPtr.Zero)
        {
            // No active window
            return;
        }

        // Get bounding rectangle in device coordinates
        var hr = NativeMethods.DwmGetWindowAttribute(
            hwnd,
            NativeMethods.DWMWA_EXTENDED_FRAME_BOUNDS,
            out NativeMethods.RECT rect,
            Marshal.SizeOf<NativeMethods.RECT>());

        if (hr != 0)
        {
            return;
        }

        int windowWidth = rect.Right - rect.Left;
        int windowHeight = rect.Bottom - rect.Top;

        if (windowWidth <= 0 || windowHeight <= 0)
        {
            return;
        }

        uint dpi = 96;

        var awareness = NativeMethods.GetAwarenessFromDpiAwarenessContext(NativeMethods.GetWindowDpiAwarenessContext(hwnd));
        bool isDpiAware = awareness is NativeMethods.DPI_AWARENESS.PER_MONITOR_AWARE;

        // If the window is DPI unaware, we need to adjust the scale based on the monitor DPI
        if (isDpiAware)
        {
            dpi = NativeMethods.GetDpiForWindow(hwnd);
        }
        else
        {
            var monitor = NativeMethods.MonitorFromWindow(hwnd, NativeMethods.MONITOR_DEFAULTTONEAREST);

            if (NativeMethods.GetDpiForMonitor(monitor, NativeMethods.MONITOR_DPI_TYPE.MDT_EFFECTIVE_DPI, out var monDpiX, out _) == 0)
            {
                dpi = monDpiX;
            }
        }

        double scale = 96.0 / dpi;

        await overlay.ShowAsync(new(rect.Left, rect.Top, windowWidth * scale, windowHeight * scale));
    }

    private void MenuExit_Click(object? sender, System.EventArgs e)
    {
        ((IClassicDesktopStyleApplicationLifetime)ApplicationLifetime!).Shutdown();
    }
}