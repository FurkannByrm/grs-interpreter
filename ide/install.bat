@echo off
REM GRS IDE Kurulum Scripti — Windows
REM ZeroBrane Studio icin GRS dil destegi kurar
REM Interpreter binary'sini PATH'e ekler

echo === GRS IDE Kurulumu ===
echo.

set SCRIPT_DIR=%~dp0
set REPO_DIR=%SCRIPT_DIR%..
set ZB_DIR=%USERPROFILE%\.zbstudio
set INTERPRETER=%REPO_DIR%\grs_interpreter\build\interpreter.exe

REM ── 1. ZeroBrane Studio dizinlerini olustur ──
if not exist "%ZB_DIR%\spec" mkdir "%ZB_DIR%\spec"
if not exist "%ZB_DIR%\api\grs" mkdir "%ZB_DIR%\api\grs"
if not exist "%ZB_DIR%\interpreters" mkdir "%ZB_DIR%\interpreters"
if not exist "%ZB_DIR%\packages" mkdir "%ZB_DIR%\packages"

REM ── 2. ZeroBrane dosyalarini kopyala ──
copy /Y "%SCRIPT_DIR%zerobrane\grs.lua"             "%ZB_DIR%\spec\" >nul
copy /Y "%SCRIPT_DIR%zerobrane\grs.api"             "%ZB_DIR%\api\grs\grs.lua" >nul
copy /Y "%SCRIPT_DIR%zerobrane\grs_interpreter.lua" "%ZB_DIR%\interpreters\" >nul
copy /Y "%SCRIPT_DIR%zerobrane\grs-support.lua"     "%ZB_DIR%\packages\" >nul

echo ZeroBrane dosyalari kuruldu:
echo   %ZB_DIR%\spec\grs.lua              (dil tanimi)
echo   %ZB_DIR%\api\grs\grs.lua           (autocomplete)
echo   %ZB_DIR%\interpreters\grs_interpreter.lua (F5 ile calistirma)
echo   %ZB_DIR%\packages\grs-support.lua  (ana yukleyici)
echo.

REM ── 3. Interpreter binary'sini kullaniciya bildir ──
if exist "%INTERPRETER%" (
    echo Interpreter bulundu: %INTERPRETER%
    echo   .grs dosyalarini herhangi bir dizinden calistirmak icin
    echo   interpreter.exe'nin bulundugu dizini PATH'e ekleyin.
) else (
    echo UYARI: Interpreter binary bulunamadi: %INTERPRETER%
    echo   Once derleyin: cd grs_interpreter\build ^& cmake .. ^& make
)

REM ── 4. Eski user.lua temizle (varsa) ──
if exist "%ZB_DIR%\user.lua" del "%ZB_DIR%\user.lua"

echo.
echo Kurulum tamamlandi. ZeroBrane Studio'yu yeniden baslatin.
pause
