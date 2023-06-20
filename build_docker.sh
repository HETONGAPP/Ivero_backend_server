#!/bin/bash

echo -e "\033[36m*********************************************************\033[0m"
echo -e "\033[36m*                                                       *\033[0m"
echo -e "\033[36m*            Installing the required dependence         *\033[0m"
echo -e "\033[36m*               This may take a while...                *\033[0m"
echo -e "\033[36m*                                                       *\033[0m"
echo -e "\033[36m*********************************************************\033[0m"


cd
git clone https://github.com/zeromq/libzmq.git
cd libzmq
mkdir build
cd build
cmake ..
sudo make -j4 install

cd
git clone https://github.com/zeromq/cppzmq.git
cd cppzmq
mkdir build
cd build
cmake ..
sudo make -j4 install



cd

current_path="$(dirname "$(readlink -f "$0")")"
project_name=Ivero_backend_server

if [ -d "install" ]; then 
    echo "install folder exist."
else 
    mkdir install
fi

echo -e "\033[36m*********************************************************\033[0m"
echo -e "\033[36m*                                                       *\033[0m"
echo -e "\033[36m*            Building the sub-module scripts            *\033[0m"
echo -e "\033[36m*               This may take a while...                *\033[0m"
echo -e "\033[36m*                                                       *\033[0m"
echo -e "\033[36m*********************************************************\033[0m"



cd /tong/$project_name/scripts
./build.sh

echo -e "\033[36m*********************************************************\033[0m"
echo -e "\033[36m*            sub-module scripts has been built          *\033[0m"
echo -e "\033[36m*********************************************************\033[0m"


echo -e "\033[36m*********************************************************\033[0m"
echo -e "\033[36m*            Cmake the Ivero backend sevrer             *\033[0m"
echo -e "\033[36m*********************************************************\033[0m"


cd ..

cmake CMakeLists.txt 

echo -e "\033[36m*********************************************************\033[0m"
echo -e "\033[36m*                                                       *\033[0m"
echo -e "\033[36m*            Building the Ivero backend sevrer          *\033[0m"
echo -e "\033[36m*               This may take a while...                *\033[0m"
echo -e "\033[36m*                                                       *\033[0m"
echo -e "\033[36m*********************************************************\033[0m"

COLOR="\033[31m"
RESET="\033[0m"
make -j4 && sudo make install


