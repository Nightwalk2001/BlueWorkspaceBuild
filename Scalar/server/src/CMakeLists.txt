set(SRC_HOME_DIR ${HOME_DIR}/src)
aux_source_directory(plugin SCALAR_SRC_LIST)
aux_source_directory(plugin/Handler SCALAR_SRC_LIST)
aux_source_directory(parser SCALAR_SRC_LIST)
aux_source_directory(GraphLine SCALAR_SRC_LIST)
aux_source_directory(GraphLine/Smoother SCALAR_SRC_LIST)
aux_source_directory(${PROJECT_ROOT_DIR}/proto SCALAR_SRC_LIST)
aux_source_directory(defs SCALAR_SRC_LIST)
aux_source_directory(FileManager SCALAR_SRC_LIST)
aux_source_directory(GraphManager SCALAR_SRC_LIST)
aux_source_directory(Util SCALAR_SRC_LIST)
set(LOG_SRC ${PROJECT_ROOT_DIR}/plugin_core/src/Logger.cpp)

list(APPEND ${PROJECT_NAME}_SRC
        ${PROTO_SRC}
        ${SCALAR_SRC_LIST}
        ${LOG_SRC})
include_directories(${SRC_HOME_DIR}
        ${SRC_HOME_DIR}/plugin
        ${SRC_HOME_DIR}/parser
        ${SRC_HOME_DIR}/GraphLine
        ${SRC_HOME_DIR}/GraphLine/SmoothingLine
        ${PROJECT_ROOT_DIR}/proto
        ${PROJECT_ROOT_DIR}
)
set(LIBRARY_OUTPUT_PATH ${LIBRARY_OUTPUT_PATH}/${PROJECT_NAME})
add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SRC}
        FileManager/File.h
        FileManager/VirtualFile.h
        FileManager/VirtualFile.cpp
        GraphLine/LineOp/SampleLineOp.h
        plugin/Handler/ScalarVisuallyTokenSet.cpp
        plugin/Handler/ScalarVisuallyTokenSet.h
        plugin/Handler/ScalarVisuallyFileMergeHandler.cpp
        plugin/Handler/ScalarVisuallyFileMergeHandler.h
        GraphLine/LineOp/LineOpFactory.h
        GraphLine/Smoother/WindowMedianSmoother.cpp
        GraphLine/Smoother/WindowMedianSmoother.h
        GraphLine/Smoother/WindowTopXSmoother.cpp
        GraphLine/Smoother/WindowTopXSmoother.h)
target_include_directories(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_H})
target_include_directories(${PROJECT_NAME} PRIVATE ${rapidjson_INC})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_ROOT_DIR}/plugin_core/include)
target_link_libraries(${PROJECT_NAME} PRIVATE mindboard::protobuf)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_ROOT_DIR}/rapidjson/include/rapidjson)

target_link_directories(${PROJECT_NAME} PUBLIC ${PROJECT_ROOT_DIR}/output/lib)
target_link_libraries(${PROJECT_NAME} PUBLIC msinsight)
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
endif ()

#--------  ut test  --------

if (ENABLE_TESTCASES OR ENABLE_CPP_ST)
    enable_testing()
    aux_source_directory(test TEST_SRC)
    aux_source_directory(${PROJECT_ROOT_DIR}/plugin_core/src TEST_SRC)
    list(APPEND ${PROJECT_NAME}_TEST_SRC
            ${${PROJECT_NAME}_SRC}
            ${TEST_SRC})
    list(APPEND ${PROJECT_NAME}_TEST_H
            ${${PROJECT_NAME}_H})
    add_executable(${PROJECT_NAME}_test ${${PROJECT_NAME}_TEST_SRC})
    target_include_directories(${PROJECT_NAME}_test PUBLIC ${${PROJECT_NAME}_TEST_H})
    target_include_directories(${PROJECT_NAME}_test PUBLIC ${PROJECT_ROOT_DIR}/plugin_core/include)
    target_include_directories(${PROJECT_NAME}_test PUBLIC ${PROJECT_ROOT_DIR})
    target_include_directories(${PROJECT_NAME}_test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/plugin)
    target_include_directories(${PROJECT_NAME}_test PRIVATE ${PROJECT_ROOT_DIR}/rapidjson/include/rapidjson)

    target_link_libraries(${PROJECT_NAME}_test PRIVATE stdc++fs)
    target_link_libraries(${PROJECT_NAME}_test PRIVATE mindboard::protobuf)
    target_link_libraries(${PROJECT_NAME}_test PUBLIC gtest_main gmock_main)
    target_link_libraries(${PROJECT_NAME}_test PRIVATE mindboard::protobuf)
    target_link_libraries(${PROJECT_NAME}_test PUBLIC ${MSINSIGHT_LIB})
#    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${PROJECT_NAME}_test PUBLIC -O0 -g)
#    endif ()

    set_target_properties(${PROJECT_NAME}_test PROPERTIES EXCLUDE_FROM_ALL true)
endif ()