@echo off
REM GRS IDE Install Script for Windows
REM Installs GRS language support files into ZeroBrane Studio user directory
REM Run this from its own location (ide\) — no administrator rights required.

setlocal EnableDelayedExpansion

set SCRIPT_DIR=%~dp0
REM Remove trailing backslash from SCRIPT_DIR
if "%SCRIPT_DIR:~-1%"=="\" set SCRIPT_DIR=%SCRIPT_DIR:~0,-1%

set REPO_DIR=%SCRIPT_DIR%\..
set ZB_SRC=%SCRIPT_DIR%\zerobrane
set ZB_DIR=%USERPROFILE%\.zbstudio
set BUILD_DIR=%REPO_DIR%\grs_interpreter\build
set GRS_STEP=%BUILD_DIR%\grs_step.exe

echo.
echo =========================================
echo   GRS IDE Installation (Windows)
echo =========================================
echo.

REM ── 1. Create ZeroBrane Studio plugin directories ──────────────────────────
if not exist "%ZB_DIR%\spec"          mkdir "%ZB_DIR%\spec"
if not exist "%ZB_DIR%\api\grs"       mkdir "%ZB_DIR%\api\grs"
if not exist "%ZB_DIR%\interpreters"  mkdir "%ZB_DIR%\interpreters"
if not exist "%ZB_DIR%\packages"      mkdir "%ZB_DIR%\packages"

REM ── 2. Copy plugin files ───────────────────────────────────────────────────
copy /Y "%ZB_SRC%\grs.lua"             "%ZB_DIR%\spec\"             >nul
copy /Y "%ZB_SRC%\grs.api"             "%ZB_DIR%\api\grs\grs.lua"   >nul
copy /Y "%ZB_SRC%\grs_interpreter.lua" "%ZB_DIR%\interpreters\"     >nul
copy /Y "%ZB_SRC%\grs-support.lua"     "%ZB_DIR%\packages\"         >nul

echo Plugin files installed to: %ZB_DIR%
echo   spec\grs.lua
echo   api\grs\grs.lua
echo   interpreters\grs_interpreter.lua
echo   packages\grs-support.lua
echo.

REM ── 3. Check interpreter binary ───────────────────────────────────────────
if exist "%GRS_STEP%" (
    echo Interpreter found: %GRS_STEP%
    echo.
    echo Adding build directory to your PATH...
    REM setx modifies the permanent user PATH (no admin needed, no /M flag)
    setx PATH "%PATH%;%BUILD_DIR%" >nul 2>&1
    if errorlevel 1 (
        echo WARNING: Could not update PATH automatically.
        echo Manually add to your PATH: %BUILD_DIR%
    ) else (
        echo PATH updated. Open a NEW terminal/ZeroBrane to apply.
    )
) else (
    echo.
    echo WARNING: grs_step.exe not found at:
    echo   %GRS_STEP%
    echo.
    echo Build the interpreter first:
    echo   1. Open MSYS2 UCRT64 terminal
    echo   2. cd ~/grs-interpreter/grs_interpreter
    echo   3. mkdir -p build ^&^& cd build
    echo   4. cmake .. -G "Unix Makefiles"
    echo   5. make -j$(nproc^)
    echo.
    echo Then run this install.bat again.
)

REM ── 4. Create user.lua if it does not exist ────────────────────────────────
if not exist "%ZB_DIR%\user.lua" (
    echo.
    echo Creating default user.lua ...
    echo -- GRS Interpreter configuration> "%ZB_DIR%\user.lua"
    echo -- Set TCP address of robot controller (leave empty for offline mode)>> "%ZB_DIR%\user.lua"
    echo -- grs = { tcp = "10.42.0.43:12345" }>> "%ZB_DIR%\user.lua"
    echo user.lua created at: %ZB_DIR%\user.lua
) else (
    echo user.lua already exists — not overwritten.
)

echo.
echo =========================================
echo   Installation complete!
echo   Restart ZeroBrane Studio to apply.
echo =========================================
echo.
pause
