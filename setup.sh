#!/bin/bash

RED="\033[31m"
BLUE="\033[34m"
GREEN="\033[32m"
RESET="\033[0m"

if [[ $EUID -ne 0 ]]; then
    echo -e "${RED}[!]Root privileges required!${RESET}" 
    echo -e "${BLUE}Re-running as sudo...${RESET}"
    sudo $0 $@
    exit $?
fi

wget -q --progress=dot "https://github.com/premake/premake-core/releases/download/v5.0.0-beta2/premake-5.0.0-beta2-linux.tar.gz" &
while pgrep -x wget >/dev/null; do
    echo -e "${GREEN}Dowloading premake...${RESET}"
    sleep 1
done

tar -xf "premake-5.0.0-beta2-linux.tar.gz"

mv ./libluasocket.so /usr/lib/
mv ./premake5 /usr/local/bin/

apt update
apt install -y libglfw3-dev libfreetype-dev libx11-dev libgtk-3-dev libasound2-dev libglu1-mesa-dev freeglut3-dev mesa-common-dev python3-glad mesa-utils

rm -rf "premake-5.0.0-beta2-linux.tar.gz" "example.so"
