if(ENABLE_GITEE OR ENABLE_GITEE_EULER) # Channel GITEE_EULER is NOT supported now, use GITEE instead.
    set(GIT_REPOSITORY "http://gitee.com/Tencent/RapidJSON.git")
    set(TAG "6089180ecb704cb2b136777798fa1be303618975")
    set(INCLUDE "./include/rapidjson")
else()
    set(GIT_REPOSITORY "http://github.com/Tencent/rapidjson.git")
    set(TAG "6089180ecb704cb2b136777798fa1be303618975")
    set(INCLUDE "./include/rapidjson")
endif()


set(ENABLE_NATIVE_JSON "off")
if(EXISTS ${TOP_DIR}/mindspore/lite/providers/json/native_json.cfg)
    set(ENABLE_NATIVE_JSON "on")
endif()
if(ENABLE_NATIVE_JSON)
    file(STRINGS ${TOP_DIR}/mindspore/lite/providers/json/native_json.cfg native_json_path)
    insight_add_pkg(rapidjson
            GIT_TAG ${TAG}
            HEAD_ONLY ${INCLUDE}
            DIR ${native_json_path})
    add_library(mindspore::json ALIAS rapidjson)
else()
    insight_add_pkg(rapidjson
            GIT_TAG ${TAG}
            HEAD_ONLY ${INCLUDE}
            GIT_REPOSITORY ${GIT_REPOSITORY}
            SHA256 ${SHA256}
    )
    include_directories(${rapidjson_INC})
    add_library(mindspore::json ALIAS rapidjson)
endif()