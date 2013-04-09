# 
# Do NOT Edit the Auto-generated Part!
# Generated by: spectacle version 0.25
# 

Name:       telepathy-ring

# >> macros
# << macros
%define keepstatic 1

Summary:    GSM connection manager for the Telepathy framework
Version:    2.2.1
Release:    2
Group:      System/Libraries
License:    LGPLv2.1
URL:        https://github.com/nemomobile/telepathy-ring/
Source0:    %{name}-%{version}.tar.bz2
Source100:  telepathy-ring.yaml
Requires:   telepathy-filesystem
Requires:   ofono
Requires:   telepathy-mission-control
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(check)
BuildRequires:  pkgconfig(libxslt)
BuildRequires:  pkgconfig(dbus-1)
BuildRequires:  pkgconfig(dbus-glib-1)
BuildRequires:  pkgconfig(uuid)
BuildRequires:  pkgconfig(telepathy-glib) >= 0.11.7
BuildRequires:  pkgconfig(mission-control-plugins)
BuildRequires:  python >= 2.5

%description
%{summary}.

%package tests
Summary:    Tests for %{name}
Group:      System/Libraries
Requires:   %{name} = %{version}-%{release}

%description tests
%{summary}.

%package devel
Summary:    Development files for %{name}
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description devel
%{summary}.


%prep
%setup -q -n %{name}

# >> setup
# << setup

%build
# >> build pre
mkdir m4 || true
# << build pre

%reconfigure 
make %{?jobs:-j%jobs}

# >> build post
# << build post

%install
rm -rf %{buildroot}
# >> install pre
# << install pre
%make_install

# >> install post
# << install post


%files
%defattr(-,root,root,-)
# >> files
%{_datadir}/dbus-1/services/*
%{_datadir}/telepathy/managers/*
%{_libexecdir}/*
%{_libdir}/mission-control-plugins.0/mcp-account-manager-ring.so
%doc %{_mandir}/man8/telepathy-ring.8.gz
# << files

%files tests
%defattr(-,root,root,-)
# >> files tests
%{_libdir}/tests/*
# << files tests

%files devel
%defattr(-,root,root,-)
# >> files devel
%{_libdir}/*.a
%{_includedir}/*
# << files devel
