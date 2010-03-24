# drm.spec
#
%define name            drm
%define version         1.2.4
%define release         1
 
Summary:	Digital Radio Mondiale (DRM) software receiver.
Name:		%{name}
Version:	%{version}
Release:	%{release}
License:	GPL
Group:		Applications/Communications
Source0:	%{name}-%{version}.tar.gz
URL:		http://drm.sourceforge.net/
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires:	autoconf, automake
BuildRequires:	fftw < 3 fftw-devel < 3
BuildRequires:	qwt >= 4.2, qwt-devel >= 4.2
BuildRequires:	faad2 >= 2.0.1, faad2-devel >= 2.0.1
BuildRequires:	libjournaline, hamlib, hamlib-devel

%description
Digital Radio Mondiale (DRM) is a new digital radio standard for
the long, medium and short wave ranges.

Dream is an open source software implementation of a DRM receiver.

%prep
%setup -q

%build
%configure --with-journaline --with-hamlib
make

%install
rm -rf %{buildroot}
%makeinstall

%clean
rm -rf %{buildroot}

%files
%defattr(-, root, root)
%doc AUTHORS ChangeLog COPYING INSTALL NEWS README TODO
%{_bindir}/drm

%changelog
* Wed Apr 24 2005 Alexander Kurpiers <kurpiers@sf.net>
- add libjournaline and hamlib
* Wed Dec 17 2003 Alexander Kurpiers <kurpiers@sf.net>
- some changes for RedHat
* Sun Nov 30 2003 Tomi Manninen <oh2bns@sral.fi>
- First try

