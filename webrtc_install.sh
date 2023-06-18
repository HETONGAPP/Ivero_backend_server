
echo "*********************************************************"
echo "*                                                       *"
echo "*              Updating the system                      *"
echo "*            This may take a while...                   *"
echo "*                                                       *"
echo "*********************************************************"

cd
# update apt get
yes | sudo apt-get update -y && yes | sudo DEBIAN_FRONTEND=noninteractive apt-get upgrade -yq  && yes | sudo DEBIAN_FRONTEND=noninteractive apt-get dist-upgrade -yq

echo "*********************************************************"
echo "*                                                       *"
echo "*          Installing the required dependences          *"
echo "*            This may take a while...                   *"
echo "*                                                       *"
echo "*********************************************************"

sudo DEBIAN_FRONTEND=noninteractive apt-get install -y \
  build-essential \
  cmake \
  git \
  pkg-config \
  wget \
  software-properties-common

sudo DEBIAN_FRONTEND=noninteractive apt-get -yq install \
  ninja-build \
  libgtk-3-dev \
  libeigen3-dev \
  libboost-all-dev \
  libmsgpack-dev \
  nlohmann-json3-dev \
  snapd \
  tmux

sudo snap install ttyd --classic

## python alias
alias python=python3
sudo ln -s /usr/bin/python3 /usr/bin/python
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y python3-msgpack
sudo apt-get install python3-pip
pip install websockets
pip install setproctitle
 
# install ROS Humble

echo "*********************************************************"
echo "*                                                       *"
echo "*          Installing the ROS2 HUMBLE                   *"
echo "*            This may take a while...                   *"
echo "*                                                       *"
echo "*********************************************************"

sudo DEBIAN_FRONTEND=noninteractive apt install software-properties-common
sudo add-apt-repository universe
sudo apt install curl -y
sudo curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key -o /usr/share/keyrings/ros-archive-keyring.gpg
echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/ros2/ubuntu $(. /etc/os-release && echo $UBUNTU_CODENAME) main" | sudo tee /etc/apt/sources.list.d/ros2.list > /dev/null
sudo apt install -y ros-humble-desktop python3-argcomplete ros-dev-tools
echo 'source /opt/ros/humble/setup.bash' >> ~/.bashrc
sudo apt install ros-humble-diagnostic-updater &&  sudo apt install ros-humble-image-transport-plugins && sudo apt install ros-humble-rosbridge-server

echo "*********************************************************"
echo "*                                                       *"
echo "*       Installing AND building the librealsense        *"
echo "*            This may take a while...                   *"
echo "*                                                       *"
echo "*********************************************************"

wget https://github.com/IntelRealSense/librealsense/raw/master/scripts/libuvc_installation.sh
chmod +x ./libuvc_installation.sh
./libuvc_installation.sh


echo "*********************************************************"
echo "*              Installing the server                    *"
echo "*********************************************************"

git clone https://github.com/HETONGAPP/server.git
cd server && cp -r sender.py /home/khadas/

echo "*********************************************************"
echo "*                                                       *"
echo "*          Installing the software packages             *"
echo "*            This may take a while...                   *"
echo "*                                                       *"
echo "*********************************************************"

cd
mkdir software
cd software

echo "*            Installing the yaml cpp                    *"
## install yaml cpp
git clone https://github.com/jbeder/yaml-cpp.git
cd yaml-cpp && mkdir build && cd build
cmake .. -DYAML_BUILD_SHARED_LIBS=ON
make -j$(nproc) && sudo make install
echo "*            yaml cpp  has been build                   *"

cd ~/software

echo "*            Installing the pangolin                    *"
## install pangolin
git clone --recursive https://github.com/stevenlovegrove/Pangolin.git
cd Pangolin
./scripts/install_prerequisites.sh recommended
cmake -B build
cmake --build build
cd build && sudo make install
cd ~/software
echo "*            Pangolin  has been build                   *"

echo "*             Installing the zipper                     *"
## install zipper
git clone --recursive https://github.com/sebastiandev/zipper.git
cd zipper && mkdir build && cd build
cmake ../
make -j$(nproc) && sudo make install
echo "*              Zipper  has been build                   *"

sudo ldconfig

echo "*********************************************************"
echo "*             Installing IveroSLAM                      *"
echo "*********************************************************"


cd
mkdir -p projects
cd projects
git clone https://github.com/HETONGAPP/IveroSLAM.git
cd IveroSLAM
chmod u+x build.sh

echo "*********************************************************"
echo "*                                                       *"
echo "*             Building IveroSLAM                        *"
echo "*            This may take a while...                   *"
echo "*                                                       *"
echo "*********************************************************"

./build.sh


echo "*********************************************************"
echo "*                                                       *"
echo "*            Installing IveroSLAM_ros2                  *"
echo "*            This may take a while...                   *"
echo "*                                                       *"
echo "*********************************************************"

cd
mkdir -p ros2_ws/src
cd ros2_ws/src
source /opt/ros/humble/setup.bash
git clone https://github.com/HETONGAPP/IveroSLAM_ros2.git
git clone https://github.com/HETONGAPP/IveroSLAM_interfaces.git
cd ..
colcon build
echo 'source /home/khadas/ros2_ws/install/setup.bash' >> ~/.bashrc


echo "*********************************************************"
echo "*        Downloading IveroSLAM_WEBRTC_SERVER            *"
echo "*********************************************************"

cd
mkdir -p workspace/src
cd workspace/src
source /opt/ros/humble/setup.bash
git clone https://github.com/HETONGAPP/webrtc_msgs.git
git clone https://github.com/HETONGAPP/webrtc_ros.git
git clone https://github.com/HETONGAPP/async_web_server_cpp.git
git clone https://github.com/HETONGAPP/pointcloud_server.git
git clone https://github.com/HETONGAPP/webrtc.git

echo "*********************************************************"
echo "*                                                       *"
echo "*          Building IveroSLAM_WEBRTC_SERVER             *"
echo "*            This may take a while...                   *"
echo "*                                                       *"
echo "*********************************************************"

cd ..
colcon build

echo 'source /home/khadas/workspace/install/setup.bash' >> ~/.bashrc




