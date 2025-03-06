if(ENABLE_GITEE OR ENABLE_GITEE_EULER) # Channel GITEE_EULER is NOT supported now, use GITEE instead.
    set(REQ_URL "https://gitee.com/mirrors/uWebSockets/repository/archive/v20.48.0.tar.gz")
    set(SHA256 "7992d0c8b11e2ec4d32cc38e8880b0b5c3115d9e3c3a0988c96df14d88b958df")
    set(VER "20.48.0")
else()
    set(REQ_URL "https://github.com/uNetworking/uWebSockets/archive/refs/tags/v20.48.0.tar.gz")
    set(SHA256 "d7455bbbf9829b3960d0478dd36ed0eba82847c4fc801416aaf89ccb7f4dfb85")
    set(VER "20.48.0")
endif()

insight_add_pkg(uWebSockets
        VER ${VER}
        HEAD_ONLY src
        URL ${REQ_URL}
        SHA256 ${SHA256})

list(APPEND MAIN_INCLUDE ${uwebsockets_INC})


