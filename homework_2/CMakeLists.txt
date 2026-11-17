# cmake_minimum_required(VERSION 3.10)
# project(homework_2 LANGUAGES CXX)

# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find_package(OpenCV REQUIRED)

# include_directories(${CMAKE_SOURCE_DIR}/include)

# add_executable(homework_2
#     src/main.cpp
#     src/Detector.cpp
# )

# target_link_libraries(homework_2 PRIVATE 
#     ${OpenCV_LIBS} 
#     opencv_aruco    # <--- 加上这个，强制链接 aruco 模块
#     opencv_core 
#     opencv_imgproc 
#     opencv_highgui 
#     opencv_videoio
# )

cmake_minimum_required(VERSION 3.10)
project(homework_2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(homework_2
    src/main.cpp
    src/Detector.cpp
)

# 直接链接所有的 OpenCV 库，避免漏库
target_link_libraries(homework_2 PRIVATE ${OpenCV_LIBS})