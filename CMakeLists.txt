# --[ Project ] ---------------------------------------------------------------
cmake_minimum_required(VERSION 3.16)
project(devtools LANGUAGES CXX)
# set(CMAKE_VERBOSE_MAKEFILE ON)

# --[ Sources ] ---------------------------------------------------------------
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
file(GLOB_RECURSE SOURCES_TO_REMOVE ${CMAKE_CURRENT_LIST_DIR}/src/_*.cpp)
if(NOT ${SOURCES_TO_REMOVE} STREQUAL "")
    list(REMOVE_ITEM SOURCES ${SOURCES_TO_REMOVE})
endif()

# --[ Execute ] ---------------------------------------------------------------
add_executable(${PROJECT_NAME} ${SOURCES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON) # -fPIC
set_target_properties(${PROJECT_NAME} PROPERTIES BUILD_RPATH ".")
# set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

# --[ Flags, Options, Defines ] -----------------------------------------------
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror)
# target_compile_definitions(${PROJECT_NAME} PRIVATE MY_DEFINE)
# target_include_directories(${PROJECT_NAME} PRIVATE src)
if(WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_link_options(${PROJECT_NAME} PRIVATE -static-libgcc -Wl,-static,-lpthread)
endif()

# --[ Logger ] ----------------------------------------------------------------
add_subdirectory(logger)
target_link_libraries(${PROJECT_NAME} logger)

# --[ Curl ] ------------------------------------------------------------------
set(BUILD_SHARED_LIBS OFF)
set(BUILD_CURL_EXE OFF)
set(BUILD_TESTING OFF)
set(HTTP_ONLY ON)
set(ENABLE_IPV6 OFF)
set(CURL_DISABLE_COOKIES ON)
set(CURL_DISABLE_CRYPTO_AUTH ON)
set(CURL_DISABLE_VERBOSE_STRINGS OFF)
set(CMAKE_USE_OPENSSL ON)
set(OPENSSL_USE_STATIC_LIBS ON)

execute_process(COMMAND cmake -E copy ${CMAKE_CURRENT_LIST_DIR}/curl/CMakeLists.txt ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists.bak)
file(READ "curl/CMakeLists.txt" FILE)
string(REGEX REPLACE "(set\\(CURL_LIBS \\\${CURL_LIBS} \"crypt32)\"\\)" "\\1;ws2_32\")" FILE "${FILE}")
file(WRITE "curl/CMakeLists.txt" "${FILE}")
add_subdirectory(curl)
execute_process(COMMAND cmake -E copy ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists.bak ${CMAKE_CURRENT_LIST_DIR}/curl/CMakeLists.txt)
target_link_libraries(${PROJECT_NAME} libcurl)

# --[ Third Party Libraries ] -------------------------------------------------
add_subdirectory(third_party_libs)
target_link_libraries(${PROJECT_NAME} zlib microtar)
