cmake_minimum_required(VERSION 3.5)
project(ivero_backend_server)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release)
ENDIF()

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -Wall   -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall   -O3")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -march=native")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")
# Add the -pthread flag to CMAKE_CXX_FLAGS
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_BUILD_TYPE "Debug")
#set(CMAKE_BUILD_TYPE "Release")


#############
## Prepare ##
#############

find_package(async_web_server_cpp REQUIRED)
#find_package(cv_bridge REQUIRED)
find_package(OpenCV REQUIRED)
find_package(webrtc REQUIRED)
find_package(X11 REQUIRED)
find_package(jsoncpp REQUIRED)
find_package(realsense2 REQUIRED)
find_package(nlohmann_json 3.2.0  REQUIRED)
find_package(ZLIB REQUIRED)
pkg_check_modules(PC_ZeroMQ QUIET zmq)

## use the hint from above to find where 'zmq.hpp' is located
find_path(ZeroMQ_INCLUDE_DIR
        NAMES zmq.hpp
        PATHS ${PC_ZeroMQ_INCLUDE_DIRS}
        )

## use the hint from above to find the location of libzmq
find_library(ZeroMQ_LIBRARY
        NAMES zmq
        PATHS ${PC_ZeroMQ_LIBRARY_DIRS}
        )

###########
## Build ##
###########

include_directories(
    include
    include/${PROJECT_NAME}
    ${OpenCV_INCLUDE_DIRS}
    ${webrtc_INCLUDE_DIRS}
)

add_definitions(${webrtc_DEFINITIONS})


set(SOURCES
    src/webrtc_json_parser.cpp
    src/ivero_backend_server.cpp
    src/webrtc_message.cpp
    src/configure_message.cpp
    src/sdp_message.cpp
    src/ice_candidate_message.cpp
    src/video_capture.cpp
    src/webrtc_client.cpp
    src/webrtc_web_server.cpp
    src/webrtc_server.cpp
    src/utility.cpp


)

set(HEADERS
  include/ivero_backend_server/webrtc_json_parser.h
  include/ivero_backend_server/webrtc_message.h
  include/ivero_backend_server/configure_message.h
  include/ivero_backend_server/sdp_message.h
  include/ivero_backend_server/ice_candidate_message.h
  include/ivero_backend_server/video_capture.h
  include/ivero_backend_server/webrtc_client.h
  include/ivero_backend_server/webrtc_web_server.h
  include/ivero_backend_server/webrtc_server.h
  include/ivero_backend_server/utility.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
  PUBLIC ${ZeroMQ_INCLUDE_DIR}
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(${PROJECT_NAME}
  async_web_server_cpp
  ${OpenCV_LIBS}
  ${realsense2_LIBRARY}
  ${webrtc_LIBRARIES}
  webrtc
  jsoncpp_lib
  realsense2::realsense2
  ${X11_LIBRARIES}
  nlohmann_json::nlohmann_json
  ZLIB::ZLIB
  dl
  ${ZeroMQ_LIBRARY}
)
set(CMAKE_INSTALL_PREFIX "install")

set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_OPTIONS "-std=c++17")

#############
## Install ##
#############
install(TARGETS
  ${PROJECT_NAME}
  DESTINATION bin/${PROJECT_NAME})
  
# Install web files
install(DIRECTORY web
  DESTINATION share/${PROJECT_NAME}
  PATTERN "Gruntfile.js" EXCLUDE
  PATTERN "node_modules" EXCLUDE
  PATTERN "package.json" EXCLUDE
  PATTERN ".gitignore" EXCLUDE
)

# Install license files
install(FILES
  LICENSE
  DESTINATION share/${PROJECT_NAME}/
)
install(FILES
  LICENSE_webrtc_adapter
  DESTINATION share/${PROJECT_NAME}/
)

install(DIRECTORY scripts
  DESTINATION share/${PROJECT_NAME}
)

install(DIRECTORY config
  DESTINATION share/${PROJECT_NAME}
)

install(DIRECTORY include/${PROJECT_NAME}
  DESTINATION include
)

