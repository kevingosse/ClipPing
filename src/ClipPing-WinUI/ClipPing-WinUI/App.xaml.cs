using Kookiz.ClipPing.Overlays;
using Microsoft.UI.Xaml;
using System;
using System.Runtime.InteropServices;
using Windows.ApplicationModel.DataTransfer;

namespace Kookiz.ClipPing;

public partial class App : Application
{
    private IOverlay? _overlay;

    /// <summary>
    /// Initializes the singleton application object.  This is the first line of authored code
    /// executed, and as such is the logical equivalent of main() or WinMain().
    /// </summary>
    public App()
    {
        InitializeComponent();
    }

    private void Clipboard_ContentChanged(object? sender, object e)
    {
        ShowOverlay();
    }

    /// <summary>
    /// Invoked when the application is launched.
    /// </summary>
    /// <param name="args">Details about the launch request and process.</param>
    protected override void OnLaunched(LaunchActivatedEventArgs args)
    {
        Clipboard.ContentChanged += Clipboard_ContentChanged;
        _overlay = LoadOverlay();
    }

    private IOverlay LoadOverlay()
    {
        // TODO: Add a way to pick what overlay to use
        return new TopOverlay();
    }

    private void ShowOverlay()
    {
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

        var dpi = NativeMethods.GetDpiForWindow(hwnd);
        double scale = 96.0 / dpi;

        _overlay!.Show(new(rect.Left, rect.Top, windowWidth * scale, windowHeight * scale));
    }
}
