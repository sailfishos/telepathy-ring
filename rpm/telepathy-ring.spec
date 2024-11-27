Name:       telepathy-ring

%define keepstatic 1

Summary:    GSM connection manager for the Telepathy framework
Version:    2.5.6
Release:    1
License:    LGPLv2+
URL:        https://github.com/sailfishos/telepathy-ring
Source0:    %{name}-%{version}.tar.bz2
Source1:    %{name}.privileges
Requires:   ofono
Requires:   telepathy-mission-control
Requires:   mapplauncherd
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(check)
BuildRequires:  pkgconfig(libxslt)
BuildRequires:  pkgconfig(dbus-1)
BuildRequires:  pkgconfig(dbus-glib-1)
BuildRequires:  pkgconfig(uuid)
BuildRequires:  pkgconfig(telepathy-glib) >= 0.11.7
BuildRequires:  pkgconfig(mission-control-plugins)
BuildRequires:  pkgconfig(libngf0) >= 0.24
BuildRequires:  pkgconfig(libdbusaccess)
BuildRequires:  pkgconfig(python3)
BuildRequires:  pkgconfig(systemd)
BuildRequires:  autoconf
BuildRequires:  automake
BuildRequires:  libtool

%description
%{summary}.

%package tests
Summary:    Tests for %{name}
Requires:   %{name} = %{version}-%{release}

%description tests
%{summary}.

%package devel
Summary:    Development files for %{name}
Requires:   %{name} = %{version}-%{release}

%description devel
%{summary}.

%package doc
Summary:   Documentation for %{name}
Requires:  %{name} = %{version}-%{release}

%description doc
Man page for %{name}.

%prep
%autosetup -p1 -n %{name}-%{version}

%build
mkdir m4 || true

%reconfigure 
%make_build

%install
%make_install
install -D -m0644 %{SOURCE1} \
        %{buildroot}%{_datadir}/mapplauncherd/privileges.d/%{name}.privileges

%files
%license COPYING
%{_userunitdir}/*
%{_datadir}/dbus-1/services/*
%{_datadir}/telepathy/managers/*
%{_libexecdir}/*
%{_libdir}/mission-control-plugins.0/mcp-account-manager-ring.so
%{_datadir}/mapplauncherd/privileges.d/%{name}.privileges

%files tests
/opt/tests/%{name}/*

%files devel
%{_libdir}/*.a
%{_includedir}/*

%files doc
%{_mandir}/man*/%{name}.*
