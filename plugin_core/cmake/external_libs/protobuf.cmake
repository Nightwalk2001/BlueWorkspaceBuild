set(protobuf_USE_STATIC_LIBS ON)
set(ENABLE_NATIVE_PROTOBUF "off")
if(EXISTS ${TOP_DIR}/mindspore/lite/providers/protobuf/native_protobuf.cfg)
    set(ENABLE_NATIVE_PROTOBUF "on")
    file(STRINGS ${TOP_DIR}/mindspore/lite/providers/protobuffer/native_protobuffer.cfg native_protobuffer_path)
endif()
if(BUILD_LITE)
    if(MSVC)
        set(protobuf_CXXFLAGS "${CMAKE_CXX_FLAGS}")
        set(protobuf_CFLAGS "${CMAKE_C_FLAGS}")
        set(protobuf_LDFLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
        set(_ms_tmp_CMAKE_STATIC_LIBRARY_PREFIX ${CMAKE_STATIC_LIBRARY_PREFIX})
        set(CMAKE_STATIC_LIBRARY_PREFIX "lib")
        if(DEBUG_MODE)
            set(protobuf_Debug ON)
        endif()
    else()
        set(protobuf_CXXFLAGS "-fstack-protector-all -Wno-maybe-uninitialized -Wno-unused-parameter \
            -fPIC -fvisibility=hidden -D_FORTIFY_SOURCE=2 -O2")
        if(NOT ENABLE_GLIBCXX)
            set(protobuf_CXXFLAGS "${protobuf_CXXFLAGS} -D_GLIBCXX_USE_CXX11_ABI=0")
        endif()
        set(protobuf_LDFLAGS "-Wl,-z,relro,-z,now,-z,noexecstack")
    endif()
else()
    if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        set(protobuf_CXXFLAGS "-fstack-protector-all -Wno-uninitialized -Wno-unused-parameter -fPIC \
            -fvisibility=hidden -D_FORTIFY_SOURCE=2 -O2")
    elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        if(MSVC)
            set(protobuf_CXXFLAGS "/DWIN32 /D_WINDOWS /W3 /GR /EHsc")
            set(protobuf_CFLAGS "${CMAKE_C_FLAGS}")
            set(protobuf_LDFLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
            set(_ms_tmp_CMAKE_STATIC_LIBRARY_PREFIX ${CMAKE_STATIC_LIBRARY_PREFIX})
            set(CMAKE_STATIC_LIBRARY_PREFIX "lib")
            if(DEBUG_MODE)
                set(protobuf_Debug ON)
            endif()
        else()
            set(protobuf_CXXFLAGS "-fstack-protector-all -Wno-maybe-uninitialized -Wno-unused-parameter \
                -fPIC -fvisibility=hidden -D_FORTIFY_SOURCE=2 -O2")
        endif()
    else()
        set(protobuf_CXXFLAGS "-fstack-protector-all -Wno-maybe-uninitialized -Wno-unused-parameter \
            -fPIC -fvisibility=hidden -D_FORTIFY_SOURCE=2 -O2")
        if(NOT ENABLE_GLIBCXX)
            set(protobuf_CXXFLAGS "${protobuf_CXXFLAGS} -D_GLIBCXX_USE_CXX11_ABI=0")
        endif()
        set(protobuf_LDFLAGS "-Wl,-z,relro,-z,now,-z,noexecstack")
    endif()
endif()

set(_ms_tmp_CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
set(CMAKE_CXX_FLAGS ${_ms_tmp_CMAKE_CXX_FLAGS})
string(REPLACE " -Wall" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REPLACE " -Werror" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

if(ENABLE_GITEE OR ENABLE_GITEE_EULER) # Channel GITEE_EULER is NOT supported now, use GITEE instead.
    set(REQ_URL "https://gitee.com/mirrors/protobufsource/repository/archive/v3.13.0.tar.gz")
    set(SHA256 "cdff020a718a6315d1f33df61e68b630b0aa3b99df4cf9187cea018e6c9b887d")
else()
    set(REQ_URL "https://github.com/protocolbuffers/protobuf/archive/v3.13.0.tar.gz")
    set(SHA256 "9b4ee22c250fe31b16f1a24d61467e40780a3fbb9b91c3b65be2a376ed913a1a")
endif()

if(BUILD_LITE)
    set(PROTOBUF_PATCH_ROOT ${TOP_DIR}/third_party/patch/protobuf)
else()
    set(PROTOBUF_PATCH_ROOT ${CMAKE_SOURCE_DIR}/third_party/patch/protobuf)
endif()

if(MSVC)
    insight_add_pkg(protobuf
            VER 3.13.0
            LIBS protobuf
            EXE protoc
            URL ${REQ_URL}
            SHA256 ${SHA256}
            CMAKE_PATH cmake/
            CMAKE_OPTION -Dprotobuf_BUILD_TESTS=OFF -Dprotobuf_BUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release
            -Dprotobuf_MSVC_STATIC_RUNTIME=OFF)
else()
    insight_add_pkg(protobuf
            VER 3.13.0
            LIBS protobuf
            EXE protoc
            URL ${REQ_URL}
            SHA256 ${SHA256}
            CMAKE_PATH cmake/
            CMAKE_OPTION -Dprotobuf_BUILD_TESTS=OFF -Dprotobuf_BUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release)
endif()
include_directories(${protobuf_INC})
add_library(mindboard::protobuf ALIAS protobuf::protobuf)
set(CMAKE_CXX_FLAGS  ${_ms_tmp_CMAKE_CXX_FLAGS})
# recover original value
if(MSVC)
    set(CMAKE_STATIC_LIBRARY_PREFIX, ${_ms_tmp_CMAKE_STATIC_LIBRARY_PREFIX})
endif()

if(ENABLE_NATIVE_PROTOBUF)
    set(PROTOC ${native_protobuffer_path}/bin/protoc)
    set(PROTOBUF_LIB ${native_protobuffer_path}/lib/libprotobuf.so.3.13.0.0)
    set(protobuf_LIBPATH ${native_protobuffer_path}/lib)
    set(protobuf_INC ${native_protobuffer_path}/include)

    include_directories(${protobuf_INC})
    message("protobuf_INC : ${protobuf_INC}")
    set(CMAKE_CXX_FLAGS  ${_ms_tmp_CMAKE_CXX_FLAGS})
endif()
add_custom_target(PROTO_GENERATE)
function(common_protobuf_generate target path c_var h_var)
    if(NOT ARGN)
        message(SEND_ERROR "Error: ms_protobuf_generate() called without any proto files")
        return()
    endif()

    set(${c_var})
    set(${h_var})

    foreach(file ${ARGN})
        get_filename_component(abs_file ${file} ABSOLUTE)
        get_filename_component(file_name ${file} NAME_WE)
        get_filename_component(file_dir ${abs_file} PATH)
        file(RELATIVE_PATH rel_path ${CMAKE_CURRENT_SOURCE_DIR} ${file_dir})

        list(APPEND ${c_var} "${path}/${file_name}.pb.cc")
        list(APPEND ${h_var} "${path}/${file_name}.pb.h")
        if(ENABLE_NATIVE_PROTOBUF)
            add_custom_command(
                    TARGET ${target}
                    #OUTPUT "${path}/${file_name}.pb.cc" "${path}/${file_name}.pb.h"
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${path}"
                    COMMAND ${CMAKE_COMMAND} -E env "LD_LIBRARY_PATH=${protobuf_LIBPATH}" ${PROTOC} -I${file_dir}
                    --cpp_out=${path} ${abs_file}
                    DEPENDS ${PROTOC} ${abs_file}
                    COMMENT "Running C++ protocol buffer compiler on ${file}" VERBATIM)
        else()
            get_target_property(PROTOC_EXEC protobuf::protoc IMPORT_LOCATION)
            execute_process(
                    RESULT_VARIABLE RETURN_CODE
                    COMMAND ${PROTOC_EXEC} -I${file_dir} --cpp_out=${path} ${abs_file}
                    OUTPUT_VARIABLE STDOUT
                    ERROR_VARIABLE STDERR
                   COMMAND_ERROR_IS_FATAL ANY
                   )
            message(STATUS "Running C++ protocol buffer compiler on ${file}")
        endif()
    endforeach()

    set_source_files_properties(${${c_var}} ${${h_var}} PROPERTIES GENERATED TRUE)
    set(${c_var} ${${c_var}} PARENT_SCOPE)
    set(${h_var} ${${h_var}} PARENT_SCOPE)
endfunction()

function(ms_protobuf_generate target path c_var h_var)
    common_protobuf_generate(${target} ${path} ${c_var} ${h_var} ${ARGN})
    set(${c_var} ${${c_var}} PARENT_SCOPE)
    set(${h_var} ${${h_var}} PARENT_SCOPE)
endfunction()
