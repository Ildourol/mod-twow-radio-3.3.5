@echo off
setlocal
echo =======================================================
echo Building Turtle WoW Radio dinput8.dll
echo =======================================================

where cl >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] cl.exe compiler not found in PATH!
    echo Please run this batch script from a 32-bit Visual Studio Developer Command Prompt:
    echo   "x86 Native Tools Command Prompt for VS 2022"
    echo or run vcvars32.bat first.
    exit /b 1
)

echo Compiling proxy_unified.cpp with /MT (Static CRT)...
cl /O2 /LD /MT proxy_unified.cpp ../bass/c/bass.lib user32.lib /link /DEF:dinput8.def /OUT:../dinput8.dll
if %errorlevel% equ 0 (
    echo.
    echo [SUCCESS] dinput8.dll built successfully into the root directory!
) else (
    echo.
    echo [ERROR] Compilation or linking failed with exit code %errorlevel%.
    exit /b %errorlevel%
)
endlocal
