# Ivero_backend_server

Ivero_backend_server is using **Realsense Camera** and  **Webrtc** to achieve the  **Real-Time** visualizing  **RGB-D** and **PointCloud**

# Ivero_backend_server pre-install

 - [x] webtrc c++ lib
```bash
git clone https://github.com/HETONGAPP/webrtc.git
cd && cd webrtc/build && cmake .. 
make -j4 && sudo make install
```
 - [x] async_web_server_cpp
```bash
git clone https://github.com/HETONGAPP/async_web_server_cpp.git
cd && cd async_web_server_cpp && mkdir build && cd build && cmake .. 
make -j4 && sudo make install
```
# Build Ivero_backend_server 
```bash
cd Ivero_backend_server 
```
```bash
chmod +x build.sh && ./build.sh
```
# RUN Ivero_backend_server 
```bash
./ivero_backend_server
```

