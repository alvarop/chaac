# Inspired by https://www.mattkeeter.com/blog/2018-01-06-versioning/

set(GIT_CMD git)
set(GIT_ARGS_TAG describe --abbrev=8 --always --dirty=+)
set(GIT_ARGS_SHA describe --match ForceNone --abbrev=8 --always --dirty=+)

execute_process(COMMAND ${GIT_CMD} ${GIT_ARGS_TAG}
                OUTPUT_VARIABLE GIT_VERSION_STR
                OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(COMMAND ${GIT_CMD} ${GIT_ARGS_SHA}
                OUTPUT_VARIABLE GIT_SHA_STR
                OUTPUT_STRIP_TRAILING_WHITESPACE)


if (GIT_VERSION_STR MATCHES "^release/fw/([a-zA-Z0-9_\\-]+)/([A-za-z0-9\\-\\+\\.]+)$")
    # Extract version info from git tag
    # Expects format release/fw/appname/vx.y.z-abcdef with optional extra stuff in the end
    set(RELEASE_APP_STR "const char *RELEASE_APP_STR = \"${CMAKE_MATCH_1}\";")
    set(VERSION_STR "const char *VERSION_STR = \"${CMAKE_MATCH_2}\";")
else()
    set(RELEASE_APP_STR "const char *RELEASE_APP_STR = \"N/A\";")
    set(VERSION_STR "const char *VERSION_STR = \"N/A\";")
endif()


set(SHA_STR "const char *GIT_SHA_STR=\"${GIT_SHA_STR}\";")

set(HEADER_STR "#include <stdint.h>\n#include <stdbool.h>\n")
string(CONCAT VERSION "${HEADER_STR}\n\n${RELEASE_APP_STR}\n${VERSION_STR}\n${SHA_STR}\n")

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/version.c)
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/version.c VERSION_)
else()
    set(VERSION_ "")
endif()

if (NOT "${VERSION}" STREQUAL "${VERSION_}")
    file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/version.c "${VERSION}")
endif()

message(STATUS "git version: ${GIT_VERSION_STR}")
