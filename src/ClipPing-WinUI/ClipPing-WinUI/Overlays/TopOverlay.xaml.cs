using Windows.Foundation;
using System.Threading.Tasks;
using WinUIEx;
using System;
using System.Diagnostics;

namespace Kookiz.ClipPing.Overlays;

public sealed partial class TopOverlay : WindowEx, IOverlay
{
    public TopOverlay()
    {
        InitializeComponent();

        var handle = this.GetWindowHandle();
        
        this.SetWindowStyle(WindowStyle.Popup);
        this.SetExtendedWindowStyle(ExtendedWindowStyle.Transparent | ExtendedWindowStyle.Layered);
    }

    public async Task ShowAsync(Rect area)
    {
        Debug.WriteLine("ShowAsync");
        this.Width = area.Width;
        this.Height = area.Height;

        this.Move((int)area.Left, (int)area.Top);
        //AppWindow.ResizeClient(new((int)area.Width, (int)area.Height));
        this.SetWindowSize(area.Width, area.Height);

        Debug.WriteLine("AppWindow.Show");
        AppWindow.Show(activateWindow: false);

        Debug.WriteLine("BeginStoryboard");
        EnterStoryboard.Completed += EnterStoryboard_Completed;
        EnterStoryboard.Begin();
        //ExitStoryboard.Completed += ExitStoryboard_Completed;
        //ExitStoryboard.Begin();

    }

    private void EnterStoryboard_Completed(object? sender, object e)
    {
        ExitStoryboard.Completed += ExitStoryboard_Completed;
        ExitStoryboard.Begin();
    }

    private void ExitStoryboard_Completed(object? sender, object e)
    {
        this.AppWindow.Hide();
    }
}
