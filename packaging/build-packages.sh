#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
VERSION="1.0.0"

check_tool() {
    if ! command -v "$1" &>/dev/null; then
        echo "ERROR: '$1' is required but not installed." >&2
        exit 1
    fi
}

build_deb() {
    echo "==> Building .deb..."
    local tmpdir
    tmpdir="$(mktemp -d)"
    trap 'rm -rf "$tmpdir"' RETURN

    local srcdir="$tmpdir/multiclip-$VERSION"
    rsync -a --exclude='.git' --exclude='build' \
        --exclude='packaging/appimage/AppDir' \
        "$PROJECT_ROOT/" "$srcdir/"

    ln -sf "$SCRIPT_DIR/debian" "$srcdir/debian"

    (cd "$srcdir" && dpkg-buildpackage -us -uc -b)

    find "$tmpdir" -name '*.deb' -exec cp {} "$PROJECT_ROOT/" \;
    echo "==> .deb written to $PROJECT_ROOT"
}

build_rpm() {
    echo "==> Building .rpm..."
    check_tool rpmbuild

    meson setup --reconfigure "$BUILD_DIR" "$PROJECT_ROOT" \
        --prefix=/usr --buildtype=release
    ninja -C "$BUILD_DIR"

    local stagedir
    stagedir="$(mktemp -d)"
    trap 'rm -rf "$stagedir"' RETURN

    DESTDIR="$stagedir" ninja -C "$BUILD_DIR" install

    mkdir -p ~/rpmbuild/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
    cp "$SCRIPT_DIR/rpm/multiclip.spec" ~/rpmbuild/SPECS/

    rpmbuild -bb ~/rpmbuild/SPECS/multiclip.spec \
        --define "_version $VERSION" \
        --define "_stagedir $stagedir" \
        --nodeps

    find ~/rpmbuild/RPMS -name 'multiclip-*.rpm' \
        -exec cp {} "$PROJECT_ROOT/" \;
    echo "==> .rpm written to $PROJECT_ROOT"
}

build_appimage() {
    echo "==> Building AppImage..."
    bash "$SCRIPT_DIR/appimage/build-appimage.sh"
}

build_flatpak() {
    echo "==> Building Flatpak..."
    check_tool flatpak-builder

    if ! flatpak info org.gnome.Platform//47 &>/dev/null; then
        echo "  Installing GNOME runtime..."
        flatpak install -y flathub org.gnome.Platform//47 org.gnome.Sdk//47
    fi

    local repo="$PROJECT_ROOT/flatpak-repo"
    local bundle="$PROJECT_ROOT/multiclip-$VERSION.flatpak"

    mkdir -p "$repo"
    ostree init --mode=archive-z2 --repo="$repo" 2>/dev/null || true

    (cd "$PROJECT_ROOT" && flatpak-builder \
        --force-clean \
        --disable-cache \
        --repo="$repo" \
        "$PROJECT_ROOT/flatpak-build" \
        "$SCRIPT_DIR/flatpak/com.harsha.multiclip.yml")

    flatpak build-bundle "$repo" "$bundle" com.harsha.multiclip
    echo "==> Written: $bundle"
}

FORMATS=("$@")
if [ ${#FORMATS[@]} -eq 0 ]; then
    FORMATS=(deb rpm appimage flatpak)
fi

for fmt in "${FORMATS[@]}"; do
    case "$fmt" in
        deb)      build_deb ;;
        rpm)      build_rpm ;;
        appimage) build_appimage ;;
        flatpak)  build_flatpak ;;
        *)
            echo "Unknown format: $fmt (valid: deb rpm appimage flatpak)" >&2
            exit 1
            ;;
    esac
done

echo "==> All requested packages built successfully."