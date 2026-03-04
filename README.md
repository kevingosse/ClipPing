# ClipPing

Visual clipboard notification for Windows. ClipPing displays a brief overlay on your active window whenever the clipboard is updated, giving you instant visual feedback that your copy operation worked.

https://github.com/user-attachments/assets/38db2a32-c694-41a0-b3d8-7532220ffee9

## Installation

### Winget (recommended)

```
winget install KevinGosse.ClipPing
```

### Installer

Download `ClipPing-Setup.exe` from the [latest release](https://github.com/kevingosse/ClipPing/releases/latest).

### Portable

Download `ClipPing.exe` from the [latest release](https://github.com/kevingosse/ClipPing/releases/latest) and run it directly. No installation required.

## Usage

ClipPing runs in the background with an icon in the system tray. Every time you copy something, a brief visual overlay flashes on your active window.

Right-click the tray icon to access:
- **Settings** - Configure overlay appearance and behavior
- **About** - Version and links
- **Exit** - Close ClipPing

Double-click the tray icon to open the settings dialog.

## Overlay types

ClipPing comes with 6 overlay styles. You can pick the one you prefer and customize the color from the settings dialog.

| Overlay | Description |
|---------|-------------|
| Top | Gradient bar at the top of the window (default) |
| Bottom | Gradient bar at the bottom |
| Left | Gradient bar on the left edge |
| Right | Gradient bar on the right edge |
| Border | Solid border around the entire window |
| Aura | Glowing gradient on all edges |

<!-- TODO: Add screenshots of each overlay type -->

## Settings

<!-- TODO: Add screenshot of the settings dialog -->

- **Overlay color** - Pick any color using the color chooser
- **Overlay type** - Select one of the 6 overlay styles
- **Preview** - Preview your settings in real-time before applying
- **Start with Windows** - Launch ClipPing automatically at login

Settings are stored in `%LOCALAPPDATA%\ClipPing\settings.ini`.

## Requirements

- Windows 10 version 1607 or later
- x64

## Building from source

Open `src/ClipPing.sln` in Visual Studio 2025 and build the Release/x64 configuration. The output is placed in the `build/` directory.

## License

[MIT](LICENSE)
