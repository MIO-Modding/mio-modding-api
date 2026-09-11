#!/usr/bin/env bash
# 
# set TARGET_FILE=%~1
TARGET_FILE=$1
# set PROJECT_NAME=%~2
PROJECT_NAME=$2

# rd "%cd%\bin\%PROJECT_NAME%"
rm -rf $(pwd)/bin/${PROJECT_NAME}
# md "%cd%\bin\%PROJECT_NAME%"
mkdir $(pwd)/bin/${PROJECT_NAME}
# xcopy "%cd%\Mod" "%cd%\bin\%PROJECT_NAME%" /E /I /H /Y
rsync -r "$(pwd)/Mod/" "$(pwd)/bin/${PROJECT_NAME}"
# copy "%TARGET_FILE%" "%cd%\bin\%PROJECT_NAME%"
cp "${TARGET_FILE}" "$(pwd)/bin/${PROJECT_NAME}"
