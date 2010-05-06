#!/usr/bin/perl

$pwd = `pwd`;
chomp $pwd;
$filename = "$pwd/config_linux_arm";

open (FHI, ">$filename") || die "$!\n";

$cmd = "export CC='arm-linux-gcc'";
print FHI $cmd."\n";
$cmd = "export CXX='arm-linux-g++ -fpermissive'";
print FHI $cmd."\n";
$cmd = "export CXXFLAGS='--static -O2 -L$pwd/fft_lib/arm -I$pwd/fft_inc -L$pwd/faad2/libfaad/.libs -I$pwd/faad2/include'";
print FHI $cmd."\n";

$cmd = "cd faad2";
print FHI $cmd."\n";
$cmd = "./configure --host=arm-linux --disable-shared --with-drm";
print FHI $cmd."\n";
$cmd = "make";
print FHI $cmd."\n";
$cmd = "cd ..";
print FHI $cmd."\n";

$cmd = "./configure --host=arm-linux --without-qtgui --disable-sound --disable-alsa --disable-simulation --without-pic  --disable-dependency-tracking  --without-faac";
print FHI $cmd."\n";

system("cat $filename");
system("chmod 755 $filename");

print "==================================================================\n";
print "DRM setup for SOC class (Minsik Cho)\n";
print "Please follow the steps:\n";
print "1) %> ./config_linux_arm\n";
print "2) %> make\n";
print "3) %> arm-linux-strip linux/drm\n";
print "3) Transfer linux/drm to the ARM simulation platform.\n";
print "==================================================================\n";

