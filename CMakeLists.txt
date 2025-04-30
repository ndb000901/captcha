cmake_minimum_required(VERSION 3.10)
project(captcha VERSION 1.0.0)
message(STATUS "Project Name: ${PROJECT_NAME}")
message(STATUS "Project Version: ${PROJECT_VERSION}")

include(ExternalProject)

option(BUILD_DEPS "Whether to build deps using build-deps.sh" ON)

if(BUILD_DEPS)
    message(STATUS "Running build-deps.sh...")
    execute_process(
        COMMAND bash ${PROJECT_SOURCE_DIR}/build-deps.sh ${PROJECT_SOURCE_DIR}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    )
endif()



include_directories(
    ${PROJECT_SOURCE_DIR}/libs/freetype/include/freetype2
    ${PROJECT_SOURCE_DIR}/libs/libpng/include
    ${PROJECT_SOURCE_DIR}/src
)

link_directories(
    ${PROJECT_SOURCE_DIR}/libs/libpng/lib
    ${PROJECT_SOURCE_DIR}/libs/freetype/lib
    ${PROJECT_BINARY_DIR}
)

file(
    GLOB
    CAPTCHA_SOURCES
    ${PROJECT_SOURCE_DIR}/src/captcha.c
    ${PROJECT_SOURCE_DIR}/src/base64.c
)

add_library(captcha_static STATIC ${CAPTCHA_SOURCES})
target_link_libraries(
  captcha_static
  PRIVATE
  ${PROJECT_SOURCE_DIR}/libs/freetype/lib/libfreetype.a
  ${PROJECT_SOURCE_DIR}/libs/libpng/lib/libpng.a
)
set_target_properties(captcha_static PROPERTIES OUTPUT_NAME "captcha")
set_target_properties(captcha_static PROPERTIES SUFFIX ".a")

# file(
#     GLOB
#     TEST_SOURCES
#     ${PROJECT_SOURCE_DIR}/src/captcha.c
#     ${PROJECT_SOURCE_DIR}/src/base64.c
#     ${PROJECT_SOURCE_DIR}/test/test.c
# )
# add_executable(test ${TEST_SOURCES})
# target_link_libraries(test PRIVATE freetype png)

# file(
#     GLOB
#     TEST_SOURCES
#     ${PROJECT_SOURCE_DIR}/test/test.c
# )
# add_executable(test ${TEST_SOURCES})
# target_link_libraries(test PRIVATE captcha png freetype bz2 z brotlidec brotlicommon)