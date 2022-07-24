@echo off
wcc -d0 -zp1 -0 -oneatx main.c
wlink file main.obj name scinfo.exe
if not exist scinfo.exe goto out
vac2 -k 0x394 scinfo.exe
sauce -preset df -title "Sound Card Info from Env" -i scinfo.exe -o x.exe -date 19980423
del scinfo.exe
ren x.exe scinfo.exe
:out
