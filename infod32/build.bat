@echo off
if exist del infod32.exe
wcl -0 -ml -oneatx infod32
if not exist infod32.exe goto end

vac2 -k 0x7361 infod32.exe
ren infod32.exe infod32.bin
sauce -preset df -date 19960904 -dt executable -title "Info-DOS32 v0.3" -i infod32.bin -o infod32.exe
del infod32.bin
copy infod32.exe c:\utils

:end
