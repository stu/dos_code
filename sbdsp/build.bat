wcl /mt /0 sbdsp
vac2 sbdsp.com
rem vacuum -5 sbdsp.com
if exist sbdsp.bin del sbdsp.bin
sauce -i sbdsp.com -o sbdsp.bin -preset df -date 20160522 -title "Stu's SoundBlaster DSP Checker"
del sbdsp.com
ren sbdsp.bin sbdsp.com