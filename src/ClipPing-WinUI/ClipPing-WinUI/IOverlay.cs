using System.Threading.Tasks;
using Windows.Foundation;

namespace Kookiz.ClipPing;

internal interface IOverlay
{
    Task ShowAsync(Rect area);
}
