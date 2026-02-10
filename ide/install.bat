@echo off
REM GRS/KRL IDE Setup Installation Script for Windows
REM This script installs GRS language support for ZeroBrane Studio and Notepad++

echo ==========================================
echo GRS/KRL IDE Setup Installation (Windows)
echo ==========================================
echo.

REM Get the script directory
set SCRIPT_DIR=%~dp0

:menu
echo Select editor(s) to install GRS support for:
echo 1) ZeroBrane Studio
echo 2) Notepad++
echo 3) Both
echo 4) Cancel
echo.
set /p choice="Enter your choice (1-4): "

if "%choice%"=="1" goto zerobrane
if "%choice%"=="2" goto notepadpp
if "%choice%"=="3" goto both
if "%choice%"=="4" goto cancel
echo Invalid choice. Please try again.
echo.
goto menu

:zerobrane
echo.
echo Installing ZeroBrane Studio support...

REM ZeroBrane config directory
set ZB_DIR=%USERPROFILE%\.zbstudio

REM Create directories if they don't exist
if not exist "%ZB_DIR%\spec" mkdir "%ZB_DIR%\spec"
if not exist "%ZB_DIR%\api" mkdir "%ZB_DIR%\api"

REM Copy files
if exist "%SCRIPT_DIR%zerobrane\grs.lua" (
    copy /Y "%SCRIPT_DIR%zerobrane\grs.lua" "%ZB_DIR%\spec\" >nul
    echo [OK] Installed GRS language spec to %ZB_DIR%\spec\
) else (
    echo [ERROR] Could not find grs.lua
    goto error
)

if exist "%SCRIPT_DIR%zerobrane\grs.api" (
    copy /Y "%SCRIPT_DIR%zerobrane\grs.api" "%ZB_DIR%\api\" >nul
    echo [OK] Installed GRS API file to %ZB_DIR%\api\
) else (
    echo [ERROR] Could not find grs.api
    goto error
)

echo [OK] ZeroBrane Studio support installed successfully!
echo [INFO] Please restart ZeroBrane Studio to use GRS language support
echo.

if "%choice%"=="3" goto notepadpp
goto success

:notepadpp
echo.
echo Installing Notepad++ support...
echo.
echo Manual installation required for Notepad++:
echo 1. Open Notepad++
echo 2. Go to Language -^> User Defined Language -^> Define your language...
echo 3. Click "Import..." button
echo 4. Select this file: %SCRIPT_DIR%scintilla\grs-language.xml
echo 5. Restart Notepad++
echo.
echo The file location is:
echo %SCRIPT_DIR%scintilla\grs-language.xml
echo.
echo Press any key to open the file location...
pause >nul
explorer "%SCRIPT_DIR%scintilla"
echo.

if "%choice%"=="1" goto success
if "%choice%"=="3" goto success

:both
goto zerobrane

:cancel
echo Installation cancelled.
goto end

:error
echo.
echo ==========================================
echo Installation failed!
echo ==========================================
pause
exit /b 1

:success
echo.
echo ==========================================
echo Installation completed!
echo ==========================================
echo.
echo For more information, see: %SCRIPT_DIR%README.md
echo.
pause

:end
