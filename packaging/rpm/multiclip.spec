Name:           multiclip
Version:        0.1.0
Release:        1%{?dist}
Summary:        MultiClip clipboard manager

License:        MIT
URL:            https://github.com/harsha/multiclip
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  meson >= 0.61
BuildRequires:  ninja-build
BuildRequires:  gcc
BuildRequires:  gtk4-devel

Requires:       gtk4

%description
MultiClip is a clipboard manager application built with GTK4.

%prep
%autosetup

%build
%meson
%meson_build

%install
%meson_install

%files
%license LICENSE
%{_bindir}/multiclip
%{_datadir}/applications/com.harsha.multiclip.desktop
%{_datadir}/icons/hicolor/256x256/apps/com.harsha.multiclip.png
%{_datadir}/metainfo/com.harsha.multiclip.metainfo.xml

%changelog
* Sat Mar 14 2026 Harshadeep Donapati <harsha@example.com> - 0.1.0-1
- Initial release
