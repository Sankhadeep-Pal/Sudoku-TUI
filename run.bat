@echo off
cd /d "%~dp0"

if exist .\sudoku.exe goto LAUNCH

echo [INFO] Compiling Sudoku for the first time...
gcc .\src\main.c .\src\welcome.c .\src\sudoku.c .\src\input.c .\src\audio.c -lwinmm -o sudoku.exe

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Compilation failed!
    pause
    exit /b %ERRORLEVEL%
)

:LAUNCH
start "" /max cmd /c "cls && .\sudoku.exe"