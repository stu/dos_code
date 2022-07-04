#
# Makefile for Watcom C/C++ 10.6
#

cc = wcc
debug = all
cflags = -mc -0 -zp1 -oneatx
lflags = option quiet
sys = dos
stack_size = 32k

all: d.exe

d.exe: d.obj dlist.obj
	echo system  $(sys)>test.lnk
	echo $(lflags)>>test.lnk
	echo option stack=$(stack_size)>>test.lnk
	echo name d.exe>>test.lnk
	echo file d, dlist>>test.lnk
	wlink @test.lnk
	@if exist d.exe vac2 d.exe
	@if exist d.exe sauce -preset d -date 20161215 -i d.exe -o d.x
	@if exist d.exe del d.exe
	@if exist d.x ren d.x d.exe

dist: d.exe
	@if exist dcode.zip del dcode.zip
	@if exist d.zip del d.zip
	zip -9 d.zip d.exe d.cfg readme.txt setenv.bat
	zip -9 dcode.zip d.c dlist.c dlist.h makefile readme.txt setenv.bat d.cfg makefile.386
	@copy d.exe c:\utils

clean: .symbolic
	@if exist main @del main
	@if exist *.obj @del *.obj
	@if exist *.bak @del *.bak
	@if exist *.sym @del *.sym
	@if exist d.exe @del d.exe

.after
	@if exist test.lnk @del test.lnk

.c.obj : .AUTODEPEND
	$(cc) $(cflags) $^&
