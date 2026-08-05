TARGET_FILE=$1
PROJECT_NAME=$2

rm -rf "$(pwd)/bin/${PROJECT_NAME}"
mkdir "$(pwd)/bin/${PROJECT_NAME}"
cp -r "$(pwd)/Mod/*" "$(pwd)/bin/${PROJECT_NAME}"
cp "${TARGET_FILE}" "$(pwd)/bin/${PROJECT_NAME}"
