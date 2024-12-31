using Windows.Foundation;
using System.Threading.Tasks;
using WinUIEx;

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

    public void Show(Rect area)
    {
        Width = area.Width;
        Height = area.Height;

        this.Move((int)area.Left, (int)area.Top);
        this.SetWindowSize(area.Width, area.Height);

        AppWindow.Show(activateWindow: false);

        EnterStoryboard.Begin();
    }

    private void EnterStoryboard_Completed(object? sender, object e)
    {
        ExitStoryboard.Begin();
    }

    private void ExitStoryboard_Completed(object? sender, object e)
    {
        AppWindow.Hide();
    }
}
