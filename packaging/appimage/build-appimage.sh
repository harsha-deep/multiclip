#!/bin/bash
# Build an AppImage for multiclip.
# Requires: meson, ninja, linuxdeploy, appimagetool
# Download linuxdeploy:  https://github.com/linuxdeploy/linuxdeploy/releases
# Download appimagetool: https://github.com/AppImage/AppImageKit/releases

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
VERSION="0.1.0"
APPDIR="$SCRIPT_DIR/AppDir"
BUILD_DIR="$SCRIPT_DIR/build-appimage"

# Clean previous build
rm -rf "$APPDIR" "$BUILD_DIR"

echo "==> Configuring with meson..."
meson setup "$BUILD_DIR" "$PROJECT_ROOT" --prefix=/usr

echo "==> Building..."
meson compile -C "$BUILD_DIR"

echo "==> Installing into AppDir..."
DESTDIR="$APPDIR" meson install -C "$BUILD_DIR"

# Copy desktop file and icon to AppDir root (required by AppImage spec)
cp "$APPDIR/usr/share/applications/com.harsha.multiclip.desktop" "$APPDIR/"
cp "$APPDIR/usr/share/icons/hicolor/256x256/apps/com.harsha.multiclip.png" "$APPDIR/"

# Create AppRun entry point
cat > "$APPDIR/AppRun" << 'EOF'
#!/bin/bash
SELF=$(readlink -f "$0")
HERE="${SELF%/*}"
export PATH="${HERE}/usr/bin:${PATH}"
export LD_LIBRARY_PATH="${HERE}/usr/lib:${HERE}/usr/lib/x86_64-linux-gnu:${LD_LIBRARY_PATH:-}"
exec "${HERE}/usr/bin/multiclip" "$@"
EOF
chmod +x "$APPDIR/AppRun"

echo "==> Bundling dependencies with linuxdeploy..."
LINUXDEPLOY="${LINUXDEPLOY:-linuxdeploy-x86_64.AppImage}"
if command -v "$LINUXDEPLOY" &>/dev/null || [ -x "./$LINUXDEPLOY" ]; then
    "${LINUXDEPLOY}" \
        --appdir "$APPDIR" \
        --desktop-file "$APPDIR/com.harsha.multiclip.desktop" \
        --icon-file "$APPDIR/com.harsha.multiclip.png"
else
    echo "WARNING: linuxdeploy not found — skipping dependency bundling."
    echo "         The AppImage may not run on systems with different library versions."
fi

echo "==> Packaging AppImage..."
APPIMAGETOOL="${APPIMAGETOOL:-appimagetool-x86_64.AppImage}"
OUTPUT="$PROJECT_ROOT/multiclip-${VERSION}-$(uname -m).AppImage"
"${APPIMAGETOOL}" "$APPDIR" "$OUTPUT"

echo "==> Done: $OUTPUT"
