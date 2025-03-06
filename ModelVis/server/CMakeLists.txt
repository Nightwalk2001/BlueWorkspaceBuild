cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 17)

project(ModelVis)

set(HOME_DIR ${PROJECT_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_ROOT_DIR}/output/plugins)
set(LIBRARY_OUTPUT_PATH ${PROJECT_ROOT_DIR}/output/plugins)
set(CMAKE_CXX_VISIBILITY_PRESET  default)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all -D_FORTIFY_SOURCE=2  -ftrapv -fstack-protector-strong -fPIE -fPIC")
if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    message(STATUS "Enable debug symbol table, change optimization level to 0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
endif ()

set(CMAKE_SKIP_RPATH  true)
if (CMAKE_SYSTEM_NAME MATCHES "windows")
    if ((NOT CMAKE_BUILD_TYPE MATCHES "Debug") AND (NOT CMAKE_BUILD_TYPE MATCHES "PROFILE"))
        message(STATUS "Build type = ${CMAKE_BUILD_TYPE}, static = enable.")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--nxcompat -Wl,--dynamicbase -s -pie -Wincompatible-pointer-types")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--nxcompat -Wl,--dynamicbase -s -pie -Wincompatible-pointer-types")
        add_link_options(-static)
    endif()
endif()

include_directories(ModelVis PRIVATE ${ModelVis_H})
include_directories(ModelVis PRIVATE ${rapidjson_INC})
include_directories(ModelVis PRIVATE ${PROJECT_ROOT_DIR}/plugin_core/include)
include_directories(ModelVis PRIVATE ${PROJECT_ROOT_DIR}/rapidjson/include/rapidjson)

add_library(ModelVis SHARED ModelVisPlugin.cpp ModelVisPlugin.h ModelVisHandlers.hpp rust_ffi.h)

target_link_directories(ModelVis PUBLIC ${PROJECT_ROOT_DIR}/output/lib)
target_link_directories(ModelVis PUBLIC ${PROJECT_ROOT_DIR}/output/plugins/ModelVis)
target_link_libraries(ModelVis PUBLIC msinsight)
target_link_libraries(ModelVis PUBLIC rustffi)

set(LIBRARY_OUTPUT_PATH ${PROJECT_ROOT_DIR}/output/plugins/ModelVis)
