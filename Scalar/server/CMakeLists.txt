cmake_minimum_required(VERSION 3.20)
project(Scalar)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.3.0)
        message(FATAL_ERROR "GCC version must be 7.3.0 and above, but found ${CMAKE_CXX_COMPILER_VERSION}")
    elseif(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 11.4.0)
        message(WARNING "GCC version ${CMAKE_CXX_COMPILER_VERSION} is greater than 11.4.0, may cause unknown problems.")
    endif()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

set(HOME_DIR ${PROJECT_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_ROOT_DIR}/output/plugins)
set(LIBRARY_OUTPUT_PATH ${PROJECT_ROOT_DIR}/output/plugins)
set(CMAKE_CXX_VISIBILITY_PRESET  default)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all -D_FORTIFY_SOURCE=2  -ftrapv -fstack-protector-strong -fPIE -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-all -D_FORTIFY_SOURCE=2 -ftrapv -fstack-protector-strong -fPIE -fPIC")
if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    message(STATUS "Enable debug symbol table, change optimization level to 0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
endif ()

set(CMAKE_SKIP_RPATH  true)
if (CMAKE_SYSTEM_NAME MATCHES "windows")
    if ((NOT CMAKE_BUILD_TYPE MATCHES "Debug") AND (NOT CMAKE_BUILD_TYPE MATCHES "PROFILE"))
        message(STATUS "Build type = ${CMAKE_BUILD_TYPE}, static = enable.")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--nxcompat -Wl,--dynamicbase -s -pie -Wincompatible-pointer-types")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--nxcompat -Wl,--dynamicbase -s -pie -Wincompatible-pointer-types")
        add_link_options(-static)
    endif()
elseif()
#    if ((NOT CMAKE_BUILD_TYPE MATCHES "Debug") AND (NOT CMAKE_BUILD_TYPE MATCHES "PROFILE"))
#        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s -pie -Wl,-z,now")
#        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s -pie -Wl,-z,now")
#    endif()
endif()


add_subdirectory(src)