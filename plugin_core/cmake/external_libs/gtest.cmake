set(gtest_CXXFLAGS "-D_FORTIFY_SOURCE=2 -O2")
set(gtest_CFLAGS "-D_FORTIFY_SOURCE=2 -O2")

set(CMAKE_OPTION
        -DBUILD_TESTING=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DBUILD_SHARED_LIBS=ON
        -DCMAKE_MACOSX_RPATH=TRUE)
if(BUILD_LITE)
    if(PLATFORM_ARM64 AND CMAKE_SYSTEM_NAME MATCHES "Android")
        set(CMAKE_OPTION -DCMAKE_TOOLCHAIN_FILE=$ENV{ANDROID_NDK}/build/cmake/android.toolchain.cmake
                -DANDROID_NATIVE_API_LEVEL=19
                -DANDROID_NDK=$ENV{ANDROID_NDK}
                -DANDROID_ABI=arm64-v8a
                -DANDROID_TOOLCHAIN_NAME=aarch64-linux-android-clang
                -DANDROID_STL=${ANDROID_STL}
                ${CMAKE_OPTION})
    endif()
    if(PLATFORM_ARM32 AND CMAKE_SYSTEM_NAME MATCHES "Android")
        set(CMAKE_OPTION -DCMAKE_TOOLCHAIN_FILE=$ENV{ANDROID_NDK}/build/cmake/android.toolchain.cmake
                -DANDROID_NATIVE_API_LEVEL=19
                -DANDROID_NDK=$ENV{ANDROID_NDK}
                -DANDROID_ABI=armeabi-v7a
                -DANDROID_TOOLCHAIN_NAME=aarch64-linux-android-clang
                -DANDROID_STL=${ANDROID_STL}
                ${CMAKE_OPTION})
    endif()
endif()

if(NOT ENABLE_GLIBCXX)
    set(gtest_CXXFLAGS "${gtest_CXXFLAGS} -D_GLIBCXX_USE_CXX11_ABI=0")
endif()

if(BUILD_LITE)
    if(ENABLE_GITEE)
        set(REQ_URL "https://gitee.com/mirrors/googletest/repository/archive/release-1.8.1.tar.gz")
        set(SHA256 "9bf1fe5182a604b4135edc1a425ae356c9ad15e9b23f9f12a02e80184c3a249c")
    else()
        set(REQ_URL "https://github.com/google/googletest/archive/release-1.8.1.tar.gz")
        set(SHA256 "9bf1fe5182a604b4135edc1a425ae356c9ad15e9b23f9f12a02e80184c3a249c")
    endif()

    insight_add_pkg(gtest
            VER 1.8.1
            DOWNLOAD_ONLY ON
            URL ${REQ_URL}
            SHA256 ${SHA256})
else()
    if(ENABLE_GITEE)
        set(REQ_URL "https://gitee.com/mirrors/googletest/repository/archive/release-1.12.1.tar.gz")
        set(SHA256 "81964fe578e9bd7c94dfdb09c8e4d6e6759e19967e397dbea48d1c10e45d0df2")
    else()
        set(REQ_URL "https://github.com/google/googletest/archive/release-1.12.1.tar.gz")
        set(SHA256 "81964fe578e9bd7c94dfdb09c8e4d6e6759e19967e397dbea48d1c10e45d0df2")
    endif()

    insight_add_pkg(gtest
            VER 1.12.1
            DOWNLOAD_ONLY ON
            URL ${REQ_URL}
            SHA256 ${SHA256})
endif()