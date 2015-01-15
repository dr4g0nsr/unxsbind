Summary: A small static library for IPv4 CIDR operations.
Name: ucidr
Version: 1.1
Release: 1
license: LGPL
Group: System Environment/Libraries
Source: http://openisp.net/source/libs/ucidr-1.1.tar.gz
URL: http://openisp.net/ucidr
Distribution: unxsVZ
Vendor: Unixservice, LLC.
Packager: Unixservice Support Group <supportgrp@unixservice.com>

%description
The ucidr program provides functions for determining
if a given IPv4 is in a given IPv4 CIDR specified block
among other basic CIDR operations.

Current version provides these functions:

unsigned ExpandCIDR4(const char *cCIDR4, char *cIPs[]);
unsigned uIpv4InCIDR4(const char *cIPv4, const char *cCIDR4);
unsigned uInCIDR4Format(const char *cCIDR4,unsigned *uIPv4,unsigned *uCIDR4Mask);
unsigned uInIpv4Format(const char *cIPv4,unsigned *uIPv4);
unsigned uGetNumIPs(char *cCIDR4);
unsigned uGetNumNets(char *cCIDR4);

%prep
%setup

%build
make

%install
make install

%clean

%files
%doc README LICENSE
/usr/lib/openisp/libucidr.a
/usr/include/openisp/ucidr.h

%changelog
* Tue Apr 12 2009 Gary Wallis <support@unixservice.com> 
- Initial RPM release for 1.1
