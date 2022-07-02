@echo off
echo Batch file example to test for hanging not bug.
echo.
echo Getting DSP version
sbdsp
echo.
sbdsp --major -q
if errorlevel 4 goto dsp4

echo Your DSP is not revision 4.
echo Your should not have the MIDI 'Hanging Note' bug.
goto end

:dsp4
sbdsp -q --minor
if errorlevel 11 set errorlevel=11
if errorlevel 12 set errorlevel=12
if errorlevel 13 set errorlevel=13

if errorlevel 13 goto hangingnote
if errorlevel 12 goto hangingnote
if errorlevel 11 goto hangingnote

echo You have a v4 DSP without the hanging note bug
goto end

:hangingnote
echo You have a v4.%errorlevel% dsp
echo You _might_ have the hanging note bug.

:end