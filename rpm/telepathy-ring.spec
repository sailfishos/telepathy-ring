Name:       telepathy-ring

%define keepstatic 1

Summary:    GSM connection manager for the Telepathy framework
Version:    2.5.2
Release:    1
Group:      System/Libraries
License:    LGPLv2+
URL:        https://git.sailfishos.org/mer-core/telepathy-ring/
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
BuildRequires:  python >= 2.5

%description
%{summary}.

%package tests
Summary:    Tests for %{name}
Requires:   %{name} = %{version}-%{release}

%description tests
%{summary}.

%package devel
Summary:    Development files for %{name}
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description devel
%{summary}.

%package doc
Summary:   Documentation for %{name}
Group:     Documentation
Requires:  %{name} = %{version}-%{release}

%description doc
Man page for %{name}.

%prep
%setup -q -n %{name}-%{version}

%build
mkdir m4 || true

%reconfigure 
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%make_install
install -D -m0644 %{SOURCE1} \
        %{buildroot}%{_datadir}/mapplauncherd/privileges.d/%{name}.privileges

%files
%defattr(-,root,root,-)
%license COPYING
%{_datadir}/dbus-1/services/*
%{_datadir}/telepathy/managers/*
%{_libexecdir}/*
%{_libdir}/mission-control-plugins.0/mcp-account-manager-ring.so
%{_datadir}/mapplauncherd/privileges.d/%{name}.privileges

%files tests
%defattr(-,root,root,-)
/opt/tests/%{name}/*

%files devel
%defattr(-,root,root,-)
%{_libdir}/*.a
%{_includedir}/*

%files doc
%defattr(-,root,root,-)
%{_mandir}/man*/%{name}.*
