cmake_minimum_required(VERSION 3.20)
project(MIOModLoader VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use vcpkg toolchain if available
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Main modding API library (winhttp.dll proxy)
add_library(winhttp SHARED
    src/dllmain.cpp
    src/modding_api.cpp
    include/modding_api.h
)

target_include_directories(winhttp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(winhttp PRIVATE
    MODDING_API_EXPORTS
)

# Link against psapi
target_link_libraries(winhttp PRIVATE
    psapi
)

# Set output name and import library
set_target_properties(winhttp PROPERTIES
    OUTPUT_NAME "winhttp"
    ARCHIVE_OUTPUT_NAME "winhttp"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# Install targets
install(TARGETS winhttp
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION bin
    ARCHIVE DESTINATION lib
)

install(FILES include/modding_api.h
    DESTINATION include
)

# Copy winhttp_orig.dll from System32 to build directory
if(WIN32)
    add_custom_command(TARGET winhttp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$ENV{WINDIR}/System32/winhttp.dll"
        "${CMAKE_BINARY_DIR}/bin/winhttp_orig.dll"
        COMMENT "Copying original winhttp.dll"
    )
    
    # Also create mods directory
    add_custom_command(TARGET winhttp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
        "${CMAKE_BINARY_DIR}/bin/mods"
        COMMENT "Creating mods directory"
    )
endif()