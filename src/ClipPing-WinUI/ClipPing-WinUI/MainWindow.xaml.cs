using WinUIEx;

namespace Kookiz.ClipPing;

public sealed partial class MainWindow : WindowEx
{
    public MainWindow()
    {
        this.InitializeComponent();
        
        var handle = this.GetWindowHandle();
        
        var extendedStyle = HwndExtensions.GetExtendedWindowStyle(handle);
        HwndExtensions.SetExtendedWindowStyle(handle, extendedStyle | ExtendedWindowStyle.Transparent | ExtendedWindowStyle.Layered);
    }
}
