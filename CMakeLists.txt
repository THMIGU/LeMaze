# THMIGU's Windows and Linux CMAKE Template
# Updated 2025-06-10

# Variables
set(PROJECT LeMaze)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -s")

# Project
cmake_minimum_required(VERSION 3.28)
project(${PROJECT})

# Resources
if (WIN32)
    enable_language(RC)
    set(RESOURCES resources/resources.rc)
endif()

# Static linking for release
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_EXE_LINKER_FLAGS "-static")
endif()

# Create executable
add_executable(${PROJECT} main.cpp ${RESOURCES})

# Trick CMAKE into readding resources
#if (WIN32)
#    add_custom_target(force_resource_rebuild ALL
#        COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_SOURCE_DIR}/${RESOURCES}
#    )
#    add_dependencies(${PROJECT} force_resource_rebuild)
#endif()

# Post-build optimization
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_command(TARGET ${PROJECT} POST_BUILD COMMAND upx --best --lzma $<TARGET_FILE:${PROJECT}>)
endif()

# Signing
if (WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_command(TARGET ${PROJECT} POST_BUILD
        COMMAND signtool sign /a /tr http://timestamp.digicert.com /td sha256 /fd sha256
        "$<TARGET_FILE:${PROJECT}>"
    )
endif()