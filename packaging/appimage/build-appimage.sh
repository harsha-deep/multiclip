#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
VERSION="1.0.0"
APPDIR="$SCRIPT_DIR/AppDir"
BUILD_DIR="$SCRIPT_DIR/build-appimage"
TOOLS_DIR="$SCRIPT_DIR/tools"

LINUXDEPLOY="$TOOLS_DIR/linuxdeploy-x86_64.AppImage"
APPIMAGETOOL="$TOOLS_DIR/appimagetool-x86_64.AppImage"


mkdir -p "$TOOLS_DIR"

if [ ! -x "$LINUXDEPLOY" ]; then
    echo "==> Downloading linuxdeploy..."
    curl -Lo "$LINUXDEPLOY" \
        "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
    chmod +x "$LINUXDEPLOY"
fi

if [ ! -x "$APPIMAGETOOL" ]; then
    echo "==> Downloading appimagetool..."
    curl -Lo "$APPIMAGETOOL" \
        "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage"
    chmod +x "$APPIMAGETOOL"
fi

ensure_runnable() {
    local tool="$1"
    local dir="$(dirname "$tool")/$(basename "$tool" .AppImage)-extracted"
    if ! "$tool" --version &>/dev/null 2>&1; then
        if [ ! -d "$dir" ]; then
            echo "  FUSE unavailable, extracting $(basename $tool)..."
            (cd "$(dirname "$tool")" && "$tool" --appimage-extract >/dev/null)
            mv "$(dirname "$tool")/squashfs-root" "$dir"
        fi
        echo "$dir/AppRun"
    else
        echo "$tool"
    fi
}

LINUXDEPLOY="$(ensure_runnable "$LINUXDEPLOY")"
APPIMAGETOOL="$(ensure_runnable "$APPIMAGETOOL")"

rm -rf "$APPDIR" "$BUILD_DIR"

echo "==> Configuring with meson..."
meson setup "$BUILD_DIR" "$PROJECT_ROOT" --prefix=/usr

echo "==> Building..."
meson compile -C "$BUILD_DIR"

echo "==> Installing into AppDir..."
DESTDIR="$APPDIR" meson install -C "$BUILD_DIR"

cp "$APPDIR/usr/share/applications/com.harsha.multiclip.desktop" "$APPDIR/"
cp "$APPDIR/usr/share/icons/hicolor/256x256/apps/com.harsha.multiclip.png" \
    "$APPDIR/com.harsha.multiclip.png"

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
"$LINUXDEPLOY" \
    --appdir "$APPDIR" \
    --desktop-file "$APPDIR/com.harsha.multiclip.desktop" \
    --icon-file "$APPDIR/com.harsha.multiclip.png" || true

echo "==> Packaging AppImage..."
OUTPUT="$PROJECT_ROOT/MultiClip-${VERSION}-x86_64.AppImage"
ARCH=x86_64 "$APPIMAGETOOL" "$APPDIR" "$OUTPUT"

echo "==> Done: $OUTPUT"