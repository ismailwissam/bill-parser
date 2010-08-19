# cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.8)

#
# 项目名称
#
PROJECT("huawei_a1" CXX)

# 设置可执行文件输出路径路径
#
SET(EXECUTABLE_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/../../release/parse_module)
SET(LIBRARY_OUTPUT_PATH     ${PROJECT_SOURCE_DIR}/../../release/parse_module)

SET(CMAKE_SHARED_LINKER_FLAGS "-mimpure-text  -Wl")

#
# 增加子项目
#
ADD_SUBDIRECTORY("src")

