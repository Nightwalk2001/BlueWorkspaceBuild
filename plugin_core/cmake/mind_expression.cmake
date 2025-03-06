set(SECURE_CXX_FLAGS "")
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    if(WIN32)
        set(SECURE_CXX_FLAGS "-fstack-protector-all")
    else()
    set(SECURE_CXX_FLAGS "-fstack-protector-all -Wl,-z,relro,-z,now,-z,noexecstack")
    endif()
endif()
set(_ms_tmp_CMAKE_CXX_FLAGS_F ${CMAKE_CXX_FLAGS})

if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
endif()

set(TOP_DIR ${CMAKE_SOURCE_DIR})
set(ENABLE_GITEE ON)

include(${CMAKE_CURRENT_SOURCE_DIR}/plugin_core/cmake/options.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/plugin_core/cmake/utils.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/plugin_core/cmake/external_libs/protobuf.cmake)
#include(${CMAKE_CURRENT_SOURCE_DIR}/plugin_core/cmake/external_libs/json.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/plugin_core/cmake/external_libs/libuv.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/plugin_core/cmake/external_libs/uSockets.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/plugin_core/cmake/external_libs/uWebSockets.cmake)


if(ENABLE_TESTCASES OR ENABLE_CPP_ST)
    include(${CMAKE_CURRENT_SOURCE_DIR}/plugin_core/cmake/external_libs/gtest.cmake)
endif()

set(CMAKE_CXX_FLAGS ${_ms_tmp_CMAKE_CXX_FLAGS_F})
