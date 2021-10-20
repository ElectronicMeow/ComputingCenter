cmake_minimum_required(VERSION 3.12)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_MODULE_PATH
        ${CMAKE_SOURCE_DIR}/cmake
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake
        ${CMAKE_MODULE_PATH}
        ${ECM_MODULE_PATH})

set(INITIALIZATIONS_SRCS
        initializations/logger.h initializations/server.h)

set(HTTP_SRCS
        http/middleware/auth.cpp
        http/middleware/CORS.cpp
        http/middleware/getArray.cpp
        http/middleware/getObject.cpp
        http/middleware/verifyJson.cpp
        http/httpData.h
        http/httpServer.cpp
        http/middleware.h
        http/httpResponse.h
        http/httpServerConfig.h
        http/util.cpp
        http/util.h
        http/httpConnection.cpp
        http/httpResponse.cpp
        http/httpRequestRouter.cpp
        http/httpRequestHandler.h
        http/httpRequest.h
        http/httpCookie.h
        http/const.h
        http/httpRequest.cpp
        http/httpData.cpp
        http/httpRequestRouter.h
        http/httpConnection.h
        http/httpServer.h)

set(SERVICES_SRCS
        services/handler.cpp
        services/handler.h)

set(SRCS
        ${INITIALIZATIONS_SRCS}
        ${HTTP_SRCS}
        ${SERVICES_SRCS}
        entry.cpp)

add_executable(${PROJECT_NAME} ${SRCS})

target_link_libraries(${PROJECT_NAME}
        ${QT_LIBRARIES}
        ZLIB::ZLIB
        MeowCryptoUtils
        qtpromise)

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
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        )
