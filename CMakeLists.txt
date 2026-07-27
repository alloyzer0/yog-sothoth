cmake_minimum_required(VERSION 3.25)

project(yog_sothoth VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)

add_library(yog_sothoth_runtime_headers INTERFACE)
target_include_directories(
    yog_sothoth_runtime_headers
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

install(DIRECTORY include/ DESTINATION include)

if(BUILD_TESTING)
    add_subdirectory(tests/abi)
endif()
