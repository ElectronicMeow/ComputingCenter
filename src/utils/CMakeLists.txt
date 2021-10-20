cmake_minimum_required(VERSION 3.12)

set(CMAKE_MODULE_PATH
        ${CMAKE_SOURCE_DIR}/cmake
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake
        ${CMAKE_MODULE_PATH}
        ${ECM_MODULE_PATH})

project(MeowCryptoUtils LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(QT_MIN_VERSION 5.6.0)
find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core Network Sql)

set(QT_LIBRARIES Qt5::Core Qt5::Network Qt5::Sql)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(GMP REQUIRED)

message("Found GMP Library: ${GMP_LIBRARIES}")

add_definitions(${QT_DEFINITIONS})

add_library(${PROJECT_NAME}
        crypto_utils.cpp
        crypto_utils.h)

target_link_libraries(${PROJECT_NAME}
        ${QT_LIBRARIES}
        ${GMP_LIBRARIES})

set_target_properties(${PROJECT_NAME}
        PROPERTIES
        AUTOMOC ON
        AUTORCC ON
        AUTOUIC ON
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
