#!/usr/bin/perl

system("touch arm/Makefile.in");
system("touch configure");
system("touch config.status");

$pwd = `pwd`;
chomp $pwd;
$infilename = "$pwd/arm/Makefile.gold";
$outfilename = "$pwd/arm/Makefile";

open (FHI, "$infilename") || die "$!\n";
open (FHO, ">$outfilename") || die "$!\n";

while(<FHI>)
{
  if(/^DEFAULT_INCLUDES.*gold_arm.*/)
  {
    printf FHO "DEFAULT_INCLUDES = -I. -I\$(srcdir) -I\$(top_builddir) -I$pwd/fft_inc -I/usr/include\n";
  }
  elsif(/^LIBS.*gold_arm.*/)
  {
    printf FHO "LIBS = -l$pwd/arm/fft_lib/libfftw.a -l$pwd/arm/fft_lib/librfftw.a\n";
  }
  else
  {
    print FHO $_;
  }
}

close (FHI);
close (FHO);

system("cp $pwd/arm/faad2/config.h $pwd/faad2");

$outfilename = "$pwd/arm/exec.sh";
open (FHO, ">$outfilename") || die "$!\n";

printf FHO "make\n";
printf FHO "armcc -c -DHAVE_CONFIG_H -I./faad2 -I$pwd/faad2/libfaad -I$pwd/faad2 -I/usr/local/packages/coware/arm-1.2/common/include/ -I/usr/include -I. -I.. ./irq.c $pwd/faad2/libfaad/bits.c $pwd/faad2/libfaad/cfft.c $pwd/faad2/libfaad/common.c $pwd/faad2/libfaad/common_dummy.c $pwd/faad2/libfaad/decoder.c $pwd/faad2/libfaad/drc.c $pwd/faad2/libfaad/drm_dec.c $pwd/faad2/libfaad/error.c $pwd/faad2/libfaad/filtbank.c $pwd/faad2/libfaad/hcr.c $pwd/faad2/libfaad/huffman.c $pwd/faad2/libfaad/ic_predict.c $pwd/faad2/libfaad/is.c $pwd/faad2/libfaad/lt_predict.c $pwd/faad2/libfaad/mdct.c $pwd/faad2/libfaad/mp4.c $pwd/faad2/libfaad/ms.c $pwd/faad2/libfaad/output.c $pwd/faad2/libfaad/pns.c $pwd/faad2/libfaad/ps_dec.c $pwd/faad2/libfaad/ps_syntax.c $pwd/faad2/libfaad/pulse.c $pwd/faad2/libfaad/rvlc.c $pwd/faad2/libfaad/sbr_dct.c $pwd/faad2/libfaad/sbr_dec.c $pwd/faad2/libfaad/sbr_e_nf.c $pwd/faad2/libfaad/sbr_fbt.c $pwd/faad2/libfaad/sbr_hfadj.c $pwd/faad2/libfaad/sbr_hfgen.c $pwd/faad2/libfaad/sbr_huff.c $pwd/faad2/libfaad/sbr_qmf.c $pwd/faad2/libfaad/sbr_syntax.c $pwd/faad2/libfaad/sbr_tf_grid.c $pwd/faad2/libfaad/specrec.c $pwd/faad2/libfaad/ssr.c $pwd/faad2/libfaad/ssr_fb.c $pwd/faad2/libfaad/ssr_ipqf.c $pwd/faad2/libfaad/syntax.c $pwd/faad2/libfaad/tns.c\n";
printf FHO "armlink  -o drm  irq.o drm-SimulationParameters.o drm-DRMSignalIO.o drm-DataIO.o drm-DrmReceiver.o drm-DrmSimulation.o drm-DrmTransmitter.o drm-InputResample.o drm-MSCMultiplexer.o drm-OFDM.o drm-Parameter.o drm-TextMessage.o drm-AMDemodulation.o drm-DataDecoder.o drm-DABMOT.o drm-MOTSlideShow.o drm-Journaline.o drm-FAC.o drm-MatlibSigProToolbox.o drm-MatlibStdToolbox.o drm-CRC.o drm-Settings.o drm-Utilities.o drm-SDCReceive.o drm-SDCTransmit.o drm-ChannelEstimation.o drm-TimeLinear.o drm-TimeWiener.o drm-ChanEstTime.o drm-IdealChannelEstimation.o drm-ChannelSimulation.o drm-BlockInterleaver.o drm-SymbolInterleaver.o drm-BitInterleaver.o drm-ConvEncoder.o drm-EnergyDispersal.o drm-MLC.o drm-Metric.o drm-ViterbiDecoder.o drm-TrellisUpdateMMX.o drm-TrellisUpdateSSE2.o drm-QAMMapping.o drm-ChannelCode.o drm-CellMappingTable.o drm-OFDMCellMapping.o drm-Resample.o drm-AudioSourceDecoder.o drm-FreqSyncAcq.o drm-TimeSync.o drm-SyncUsingPil.o drm-TimeSyncTrack.o drm-main.o drm-sound.o bits.o cfft.o common.o common_dummy.o decoder.o drc.o drm_dec.o error.o filtbank.o hcr.o huffman.o ic_predict.o is.o lt_predict.o mdct.o mp4.o ms.o output.o pns.o ps_dec.o ps_syntax.o pulse.o rvlc.o sbr_dct.o sbr_dec.o sbr_e_nf.o sbr_fbt.o sbr_hfadj.o sbr_hfgen.o sbr_huff.o sbr_qmf.o sbr_syntax.o sbr_tf_grid.o specrec.o ssr.o ssr_fb.o ssr_ipqf.o syntax.o tns.o $pwd/arm/fft_lib/libfftw.a $pwd/arm/fft_lib/librfftw.a\n";

close (FHO);

system("chmod 755 $outfilename");

print "==================================================================\n";
print "DRM.ARM setup for SOC class (Minsik Cho)\n";
print "Please follow the steps\n";
print "0) Remeber to setup ARM environment\n";
print "1) %>cd arm\n";
print "2) %>./exec.sh (ignore warnings)\n";
print "3) %>armsd\n";
print "4) armsd: load drm\n";
print "5) armsd: go\n";
print "6) armsd: quit\n";
print "7) cmp RTL_ModeB_10kHz_gold.wav dummy.wav\n";
print "8) For longer input files, edit common/GUI-QT/main.cpp\n";
print "==================================================================\n";


