#!/bin/bash
# Build all package formats for multiclip.
#
# Usage:
#   ./packaging/build-packages.sh [deb] [rpm] [appimage] [flatpak]
#
# With no arguments, all formats are built.
# Tools required per format:
#   deb      — dpkg-buildpackage, debhelper, libgtk-4-dev
#   rpm      — rpmbuild, gtk4-devel
#   appimage — linuxdeploy, appimagetool (see packaging/appimage/build-appimage.sh)
#   flatpak  — flatpak-builder, org.gnome.Platform//47, org.gnome.Sdk//47

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
VERSION="0.1.0"

build_deb() {
    echo "==> Building .deb..."
    local tmpdir
    tmpdir="$(mktemp -d)"
    trap 'rm -rf "$tmpdir"' RETURN

    # Copy source tree into a versioned directory (required by dpkg-buildpackage)
    local srcdir="$tmpdir/multiclip-$VERSION"
    rsync -a --exclude='.git' --exclude='packaging/build' \
        --exclude='packaging/appimage/AppDir' --exclude='packaging/appimage/build-appimage' \
        "$PROJECT_ROOT/" "$srcdir/"

    # Symlink the debian dir into the expected location
    ln -sf "$SCRIPT_DIR/debian" "$srcdir/debian"

    (cd "$srcdir" && dpkg-buildpackage -us -uc -b)

    find "$tmpdir" -name '*.deb' -exec cp {} "$PROJECT_ROOT/" \;
    echo "==> .deb written to $PROJECT_ROOT"
}

build_rpm() {
    echo "==> Building .rpm..."
    local tarball="$PROJECT_ROOT/multiclip-$VERSION.tar.gz"

    # Create source tarball from git (excludes untracked/build files)
    (cd "$PROJECT_ROOT" && git archive --format=tar.gz --prefix="multiclip-$VERSION/" \
        -o "$tarball" HEAD)

    mkdir -p ~/rpmbuild/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
    cp "$tarball" ~/rpmbuild/SOURCES/
    cp "$SCRIPT_DIR/rpm/multiclip.spec" ~/rpmbuild/SPECS/

    rpmbuild -bb ~/rpmbuild/SPECS/multiclip.spec

    find ~/rpmbuild/RPMS -name 'multiclip-*.rpm' -exec cp {} "$PROJECT_ROOT/" \;
    rm -f "$tarball"
    echo "==> .rpm written to $PROJECT_ROOT"
}

build_appimage() {
    echo "==> Building AppImage..."
    bash "$SCRIPT_DIR/appimage/build-appimage.sh"
}

build_flatpak() {
    echo "==> Building Flatpak..."
    local repo="$PROJECT_ROOT/flatpak-repo"
    local bundle="$PROJECT_ROOT/multiclip-$VERSION.flatpak"

    flatpak-builder --force-clean --repo="$repo" \
        "$PROJECT_ROOT/flatpak-build" \
        "$SCRIPT_DIR/flatpak/com.harsha.multiclip.yml"

    flatpak build-bundle "$repo" "$bundle" com.harsha.multiclip
    echo "==> Flatpak bundle written to $bundle"
}

# Parse arguments — default to all formats
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
        *)        echo "Unknown format: $fmt (valid: deb rpm appimage flatpak)" >&2; exit 1 ;;
    esac
done

echo "==> All requested packages built successfully."
