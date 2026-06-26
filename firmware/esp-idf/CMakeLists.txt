cmake_minimum_required(VERSION 3.16)

# Pclika MCP Platform — ESP-IDF root CMakeLists
# Target: ESP32-S3
# Toolchain: ESP-IDF v5.x

set(EXTRA_COMPONENT_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/components"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(pclika_platform)
