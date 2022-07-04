@echo off
nasm e820.asm -o e820.com

nasm cpu.asm -f obj -o cpu.obj
wcl -0 -ecd -ml -oneatx e820lst.c cpu.obj

vac2 e820.com
vac2 e820lst.exe
sauce -preset df -i e820lst.exe -o xx -group "[AIH]" -title "E820 Table List" -date 19941103
del e820lst.exe
ren xx e820lst.exe