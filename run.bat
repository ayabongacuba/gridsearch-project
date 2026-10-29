@echo off
REM run.bat -- Windows wrapper.  usage:  run dfs m05
if "%1"=="" ( echo usage: run ^<stage^> [maze] & exit /b 1 )
set STAGE=%1
set MAZE=%2
if "%MAZE%"=="" set MAZE=m01-perfect-21
mingw32-make bin/%STAGE% || exit /b 1
bin\%STAGE%.exe --viz mazes\%MAZE%.grid
