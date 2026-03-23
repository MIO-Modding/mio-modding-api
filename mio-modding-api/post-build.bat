@echo off
setlocal Enabledelayedexpansion

set "outputPath=%~1"
set "modSourcePath=%~2"
set "outputDll=%~3"
set "modOutputPath=%~4"

:: Clean and recreate directory
if exist "%modOutputPath%" rd "%modOutputPath%" /S /Q
xcopy "%modSourcePath%" "%modOutputPath%" /E /I /Y

:: Define the "array"
set "outputFiles[0]=%outputDll%"
set "last_index=0"

:: The Loop
for /L %%i in (0, 1, %last_index%) do (
    :: Use 'call' or delayed expansion to resolve the nested variable
    set "currentFile=!outputFiles[%%i]!"

    set "fileOutputPath=%outputPath%!currentFile!"
    set "fileOutputTargetPath=%modOutputPath%!currentFile!"

    copy "!fileOutputPath!" "!fileOutputTargetPath!" /Y
)

pause
