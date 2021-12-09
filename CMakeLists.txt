cmake_minimum_required(VERSION 2.5)
project(muduo)

#muduo 最终编译成so动态库，设置动态库的路径，放在根目录的lib文件夹下面
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
#设置调试信息
set(CMAKE_CXX_FLAGES "${CMAKE_CXX_FLAGS} -g -std=c++11 -fPIC")

#定义参与编译的源文件
aux_source_directory(. SRC_LIST)
#编译动态库
add_library(minimuduo SHARED ${SRC_LIST})