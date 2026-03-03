cmake_minimum_required(VERSION 3.20)
project(MIOModdingAPI VERSION 0.0.3 LANGUAGES CXX)

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
 "include/flamby_handling.h" "src/flamby_handling.cpp" "include/gin_serializer.h" "src/gin_serializer.cpp")

target_include_directories(winhttp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(winhttp PRIVATE
    MODDING_API_EXPORTS
)


include(FetchContent)

FetchContent_Declare(
    zstd
    GIT_REPOSITORY https://github.com/facebook/zstd.git
    GIT_TAG        v1.5.6
    SOURCE_SUBDIR  build/cmake
)
set(ZSTD_BUILD_SHARED OFF CACHE INTERNAL "")
set(ZSTD_BUILD_STATIC ON CACHE INTERNAL "")
FetchContent_MakeAvailable(zstd)
FetchContent_Declare(
  lz4
  GIT_REPOSITORY https://github.com/lz4/lz4.git
  GIT_TAG        v1.10.0
  SOURCE_SUBDIR  build/cmake
)
set(LZ4_BUILD_CLI OFF CACHE INTERNAL "")
set(LZ4_BUILD_TESTS OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(lz4)
FetchContent_Declare(
  hash-library
  GIT_REPOSITORY https://github.com/stbrumme/hash-library.git
  GIT_TAG        master
)
FetchContent_GetProperties(hash-library)
if(NOT hash-library_POPULATED)
  FetchContent_Populate(hash-library)
  add_library(hash-library STATIC 
      "${hash-library_SOURCE_DIR}/md5.cpp"
  )
  target_include_directories(winhttp PRIVATE ${hash-library_SOURCE_DIR})
endif()

# Link against libraries
target_link_libraries(winhttp PRIVATE
    psapi
    dwmapi
    libzstd_static
    lz4_static
    hash-library
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

# Create mods directory
add_custom_command(TARGET winhttp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
    "${CMAKE_BINARY_DIR}/bin/mods"
    COMMENT "Creating mods directory"
)