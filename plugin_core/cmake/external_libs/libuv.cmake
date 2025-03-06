if(ENABLE_GITEE OR ENABLE_GITEE_EULER) # Channel GITEE_EULER is NOT supported now, use GITEE instead.
    set(REQ_URL "https://gitee.com/mirrors/libuv/repository/archive/v1.48.0.tar.gz")
    set(SHA256 "9b833b426922e2eb568631aa11bdac526e556627b1cbfbc196e4674adc3d43af")
    set(VER "1.48.0")
else()
    set(REQ_URL "https://github.com/libuv/libuv/archive/refs/tags/v1.48.0.tar.gz")
    set(SHA256 "8c253adb0f800926a6cbd1c6576abae0bc8eb86a4f891049b72f9e5b7dc58f33")
    set(VER "1.48.0")
endif()

add_compile_definitions("LIBUS_NO_SSL")
add_compile_definitions("UWS_NO_ZLIB")
add_compile_definitions("LIBUS_USE_LIBUV")

insight_add_pkg(libuv
        VER ${VER}
        DOWNLOAD_ONLY ON
        URL ${REQ_URL}
        SHA256 ${SHA256})

set_target_properties(uv PROPERTIES EXCLUDE_FROM_ALL true)