Use this in your autoexec.bat to run something once per day.

So first time you boot, run extra things, if you reboot that day, they
dont get run again.

example autoexec.bat

@echo off
once.com
if errorlevel == 1 goto firstrun
goto secondrun

:firstrun
rem do stuff only once a day
goto out

:secondrun
rem do stuff every bootup
goto out

:out
rem continue onward

