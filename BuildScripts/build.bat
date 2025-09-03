@echo off

REM Set default values for configuration and platform
set CONFIG=Debug
set PLATFORM=

REM Get solution file path
set SOLUTION=Nuake.sln

REM Check if Configuration is provided
if not "%~2"=="" (
    set CONFIG=%~2
)

REM Check if Platform is provided
if not "%~3"=="" (
    set PLATFORM=%~3
)

REM Build the solution
echo Building solution "%SOLUTION%" with Configuration=%CONFIG% and Platform=%PLATFORM%...
"C:\Program Files\Microsoft Visual Studio\2022\Community\Msbuild\Current\Bin\MSBuild.exe" "Nuake.sln" -verbosity:minimal
PAUSE
REM Check if build succeeded
if %ERRORLEVEL%==0 (
    echo Build succeeded.
) else (
    echo Build failed.
    exit /b 1
)