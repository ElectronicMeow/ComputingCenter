cmake_minimum_required(VERSION 3.12)

set(CMAKE_MODULE_PATH
        ${CMAKE_SOURCE_DIR}/cmake
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake
        ${CMAKE_MODULE_PATH}
        ${ECM_MODULE_PATH})

project(MeowComputingCenter LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(QT_MIN_VERSION 5.6.0)
find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core Network)

set(QT_LIBRARIES Qt5::Core Qt5::Network)

add_definitions(${QT_DEFINITIONS})

find_package(ZLIB)

add_subdirectory(3rdparty/qtpromise)
add_subdirectory(src/utils)

add_subdirectory(src)
