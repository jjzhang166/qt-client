Name: postbooks
Version: 4.8.2
Release: 1%{?dist}
Summary: xTuple Accounting/ERP suite desktop client
License: CPAL
Url: http://www.xtuple.com/postbooks/
Source: https://github.com/xtuple/qt-client/archive/v%version.tar.gz
BuildRequires: desktop-file-utils
BuildRequires: qt-devel
BuildRequires: xtuple-openrpt-devel
BuildRequires: xtuple-csvimp-devel
BuildRequires: qtwebkit-devel
BuildRequires: qt-assistant-adp-devel
BuildRequires: libsqlite3x-devel
Requires: qt-postgresql
Requires: qt-assistant-adp
Requires: %{name}-libs%{?_isa} = %{version}-%{release}

%global _docdir_fmt %{name}

%description
A full-featured, fully-integrated business management system, the core of
the award winning xTuple ERP Suite. Built with the open source PostgreSQL
database and the open source Qt framework for C++, it provides the
ultimate in power and flexibility for a range of businesses and
industries of any size.
This package contains the GUI.  Install it on any workstation that
needs access to the PostBooks database.  There is no server component
other than an instance of PostgreSQL itself.

%package libs
Summary: Shared libraries for PostBooks

%description libs
A full-featured, fully-integrated business management system, the core of
the award winning xTuple ERP Suite. Built with the open source PostgreSQL
database and the open source Qt framework for C++, it provides the
ultimate in power and flexibility for a range of businesses and
industries of any size.
This package provides the core libraries: libpostbooks

%package devel
Summary: PostBooks development files
Requires: %{name}-libs%{?_isa} = %{version}-%{release}
Requires: qt-devel
Requires: libdmtx-devel
Requires: xtuple-openrpt-devel
Requires: xtuple-csvimp-devel

%description devel
A full-featured, fully-integrated business management system, the core of
the award winning xTuple ERP Suite. Built with the open source PostgreSQL
database and the open source Qt framework for C++, it provides the
ultimate in power and flexibility for a range of businesses and
industries of any size.
This package provides the header files used by developers.

%prep
%setup -q -n qt-client-%version

%build
export OPENRPT_HEADERS=%{_includedir}/openrpt
export OPENRPT_LIBDIR=%{_libdir}
export OPENRPT_IMAGE_DIR=/usr/share/openrpt/OpenRPT/images
export USE_SHARED_OPENRPT=1
export CSVIMP_HEADERS=%{_includedir}/csvimp
export CSVIMP_LIBDIR=%{_libdir}
export BUILD_XTCOMMON_SHARED=1
# FIXME: find will not fail on error!
find . -name '*.ts' -exec lrelease-qt4 {} \;
qmake-qt4 .
make %{?_smp_mflags}

%install
# make install doesn't do anything for this qmake project so we do
# the installs manually
#make INSTALL_ROOT=%{buildroot} install
#rm -f %{buildroot}%{_libdir}/lib*.a
mkdir -p %{buildroot}%{_bindir}
install bin/* %{buildroot}%{_bindir}
ln -s %{_bindir}/xtuple %{buildroot}%{_bindir}/postbooks
mkdir -p %{buildroot}%{_libdir}
cp -dp lib/lib*.so* %{buildroot}%{_libdir}
find %{buildroot}%{_libdir} -name 'lib*.so*' -exec chmod 0755 {} \;
mkdir -p %{buildroot}%{_includedir}/xtuple
find common -name '*.h' -exec install -m 0644 -D {} %{buildroot}%{_includedir}/xtuple/{} \;
install -m 0644 -D guiclient/images/icon128x128.png %{buildroot}%{_datadir}/postbooks/images/icon128x128.png
mkdir -p %{buildroot}%{_datadir}/applications
desktop-file-install --dir=%{buildroot}%{_datadir}/applications *.desktop
install -m 0644 -D postbooks.appdata.xml %{buildroot}%{_datadir}/appdata/postbooks.appdata.xml

%post libs -p /sbin/ldconfig

%postun libs -p /sbin/ldconfig

%files 
%license LICENSE.txt
%dir %{_datadir}/postbooks
%dir %{_datadir}/postbooks/images
%{_datadir}/postbooks/images/icon128x128.png
%{_bindir}/*
%{_datadir}/applications/*.desktop
%{_datadir}/appdata/postbooks.appdata.xml

%files libs
%{_libdir}/lib*.so.*

%files devel
%dir %{_includedir}/xtuple/
%{_includedir}/xtuple/*
%{_libdir}/lib*.so

%changelog
* Wed Feb 25 2015 Daniel Pocock <daniel@pocock.pro> - 4.8.2-1
- Initial RPM packaging.

