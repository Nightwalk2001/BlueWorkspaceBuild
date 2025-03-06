#set(REQ_URL "https://github.com/uNetworking/uSockets/archive/refs/tags/v0.8.6.tar.gz")
#set(SHA256 "16eba133dd33eade2f5f8dd87612c04b5dd711066e0471c60d641a2f6a988f16")
#set(VER "0.8.6")
#
#insight_add_pkg(uSockets
#        VER ${VER}
#        HEAD_ONLY src
#        URL ${REQ_URL}
#        SHA256 ${SHA256})
set(usockets_INC ${PROJECT_ROOT_DIR}/uSockets-0.8.6/src)
list(APPEND MAIN_INCLUDE ${usockets_INC})

aux_source_directory(${usockets_INC} U_SOCKETS_SRC)
aux_source_directory(${usockets_INC}/crypto U_SOCKETS_SRC)
aux_source_directory(${usockets_INC}/eventing U_SOCKETS_SRC)
aux_source_directory(${usockets_INC}/internal U_SOCKETS_SRC)

