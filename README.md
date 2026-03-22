## Dependencies

### Runtime
| Library | Version | Purpose |
|---------|---------|---------|
| GTK4 | ≥ 4.6 | UI toolkit |
| GLib2 | ≥ 2.72 | Core utilities |
| SQLite3 | ≥ 3.0 | History storage |
| xdotool | any | Auto-paste (X11) |

### Install on Debian/Ubuntu/Mint
```bash
sudo apt install libgtk-4-dev libsqlite3-dev xdotool
```

### Install on Fedora/CentOS Stream
```bash
sudo dnf install gtk4-devel sqlite-devel xdotool
```

### Build tools
```bash
# Debian/Ubuntu/Mint
sudo apt install meson ninja-build gcc pkg-config

# Fedora/CentOS
sudo dnf install meson ninja-build gcc pkg-config
```

### Tray icon support
- **XFCE**: In Panel Preferences → Items, replace *Notification Area* with *Status Tray Plugin*
- **GNOME**: Install the [AppIndicator extension](https://extensions.gnome.org/extension/615/appindicator-support/)
- **KDE Plasma**: Works out of the box

## Platform support
| Platform | Status |
|----------|--------|
| Linux (X11) | [OK] Full support |
| Linux (Wayland/XWayland) | [OK] Supported |
| Windows | [NO] Not supported |
| macOS | [NO] Not supported |