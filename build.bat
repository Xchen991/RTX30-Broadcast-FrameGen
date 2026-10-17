@echo off
setlocal enabledelayedexpansion

echo [*] Locating Visual Studio build environment...

set "VCVARS="
if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "%ProgramFiles%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=%ProgramFiles%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
)

if "%VCVARS%"=="" (
    echo [-] Error: vcvars64.bat not found. Please install Visual Studio 2022 C++ build tools.
    pause
    exit /b 1
)

call "%VCVARS%" >nul 2>&1
if not exist "bin" mkdir "bin"

echo [*] Compiling version.dll (x64 Release)...
cl.exe /utf-8 /O2 /GL /EHa /LD "src\dllmain.cpp" /Fe:"bin\version.dll" /link /safeseh:no /LTCG /OPT:REF /OPT:ICF

if exist "bin\version.dll" (
    del /f /q bin\*.obj bin\*.lib bin\*.exp >nul 2>&1
    echo [+] Build successful: bin\version.dll
) else (
    echo [-] Build failed.
)

pause