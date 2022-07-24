@echo off
if "%1"=="" goto help
if "%2"=="" goto help

if exist %1.rdf goto ok1
if exist %1 goto ok2

echo input RDF file does not exist
goto out

:ok1
if exist %2 del %2>nul
copy /b rdfl_v4.com+%1.rdf %2>nul
goto ok3

:ok2
if exist %2 del %2>nul
copy /b rdfl_v4.com+%1 %2>nul
goto ok3

:ok3
echo Linked to %2
goto out

:help
echo link IN.RDF OUT.COM
goto out

:out
