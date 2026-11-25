@echo off
cd /d "%~dp0"
set "PATH=C:\msys64\ucrt64\bin;%~dp0build;%PATH%"
echo Launching Group 3 Final Project (2.5D Narrative Adventure)...
start "" "%~dp0build\NarrativeGame.exe"
