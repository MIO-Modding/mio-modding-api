set TARGET_FILE=%~1
set PROJECT_NAME=%~2

rd "%cd%\bin\%PROJECT_NAME%"
md "%cd%\bin\%PROJECT_NAME%"
xcopy "%cd%\Mod" "%cd%\bin\%PROJECT_NAME%" /E /I /H /Y
copy "%TARGET_FILE%" "%cd%\bin\%PROJECT_NAME%"
