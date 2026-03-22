Name:    multiclip
Version: %{_version}
Release: 1%{?dist}
Summary: Multi-entry clipboard manager
License: MIT
URL:     https://github.com/harsha-deep/multiclip

# No Source, no BuildRequires — binary is pre-built outside rpmbuild

Requires: gtk4, sqlite-libs, xdotool

%description
MultiClip stores your clipboard history and lets you paste
any previous entry with a single click or global hotkey (Super+V).

%install
cp -a %{_stagedir}/. %{buildroot}/

%files
%{_bindir}/multiclip
%{_datadir}/applications/com.harsha.multiclip.desktop
%{_datadir}/icons/hicolor/256x256/apps/com.harsha.multiclip.png
%{_datadir}/metainfo/com.harsha.multiclip.metainfo.xml

%changelog
* Mon Mar 23 2026 Harsha Deep <harsha@example.com> - 0.1.0-1
- Initial release