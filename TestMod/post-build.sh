TARGET_FILE=$1
PROJECT_NAME=$2

rm -rf "./bin/${PROJECT_NAME}"
mkdir "./bin/${PROJECT_NAME}"
rsync -r "./Mod/" "./bin/${PROJECT_NAME}"
cp "${TARGET_FILE}" "./bin/${PROJECT_NAME}"
