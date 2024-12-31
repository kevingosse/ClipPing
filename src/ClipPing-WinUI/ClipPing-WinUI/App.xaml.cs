using Kookiz.ClipPing.Overlays;
using Microsoft.UI.Xaml;
using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using Windows.ApplicationModel.DataTransfer;

namespace Kookiz.ClipPing;

public partial class App : Application
{
    private Window? m_window;
    private IOverlay _overlay;

    /// <summary>
    /// Initializes the singleton application object.  This is the first line of authored code
    /// executed, and as such is the logical equivalent of main() or WinMain().
    /// </summary>
    public App()
    {
        InitializeComponent();

        Clipboard.ContentChanged += Clipboard_ContentChanged;
    }

    private void Clipboard_ContentChanged(object? sender, object e)
    {
        Debug.WriteLine("Clipboard_ContentChanged");
        _ = ShowOverlay();
    }

    /// <summary>
    /// Invoked when the application is launched.
    /// </summary>
    /// <param name="args">Details about the launch request and process.</param>
    protected override void OnLaunched(LaunchActivatedEventArgs args)
    {
        m_window = new MainWindow();
        m_window.Activate();
        _overlay = LoadOverlay();
    }

    private IOverlay LoadOverlay()
    {
        // TODO: Add a way to pick what overlay to use
        return new TopOverlay();
    }

    private async Task ShowOverlay()
    {
        //var overlay = LoadOverlay();

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

        //scale = 1;

        Debug.WriteLine($"Window: {windowWidth}x{windowHeight} DPI: {dpi} Scale: {scale}");

        await _overlay.ShowAsync(new(rect.Left, rect.Top, windowWidth * scale, windowHeight * scale));
    }
}
