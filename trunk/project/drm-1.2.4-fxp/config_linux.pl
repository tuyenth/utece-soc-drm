#!/usr/bin/perl

$pwd = `pwd`;
chomp $pwd;
$filename = "$pwd/config_linux_soc";

open (FHI, ">$filename") || die "$!\n";

$cmd = "export CC='gcc -m32'";
print FHI $cmd."\n";
$cmd = "export CXX='g++ -m32 -fpermissive'";
print FHI $cmd."\n";
$cmd = "export CXXFLAGS='-pg -g -O2 -L$pwd/fft_lib -I$pwd/fft_inc -L$pwd/faad2/libfaad/.libs -I$pwd/faad2/include'";
print FHI $cmd."\n";

$cmd = "cd faad2";
print FHI $cmd."\n";
$cmd = "./configure --with-drm";
print FHI $cmd."\n";
$cmd = "make";
print FHI $cmd."\n";
$cmd = "cd ..";
print FHI $cmd."\n";

$cmd = "./configure --without-qtgui --disable-sound --disable-alsa --disable-simulation --without-pic  --disable-dependency-tracking  --without-faac";
print FHI $cmd."\n";

system("cat $filename");
system("chmod 755 $filename");

print "==================================================================\n";
print "DRM setup for SOC class (Minsik Cho)\n";
print "Please follow the steps\n";
print "1) %>./config_linux_soc\n";
print "2) %>make\n";
print "3) %>cd linux\n";
print "4) %>./drm\n";
print "5) %>cmp RTL_ModeB_10kHz_gold.wav dummy.wav\n";
print "6) You shouldn't see any msg from 5).\n";
print "==================================================================\n";

