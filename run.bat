@echo off
REM run.bat -- Windows wrapper. Usage: run <stage> [map]
if "%1"=="" ( echo usage: run ^<stage^> [map] & exit /b 1 )
set STAGE=%1
set MAP=%2
if "%MAP%"=="" set MAP=m01-perfect-21
mingw32-make bin/%STAGE% || exit /b 1
bin\%STAGE%.exe --viz maps\%MAP%.grid
