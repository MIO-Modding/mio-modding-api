# MIO Modding API
An API for modding MIO: Memories in Orbit

## Prerequisites

- CMake 3.20 or higher
- MSVC (Visual Studio 2019 or later)
- vcpkg
- Ninja
  

## Building

### Using CMake Presets
```powershell
# Configure
cmake --preset x64-release

# Build
cmake --build --preset x64-release

# Install (optional)
cmake --install build/x64-release
```


## Installation to Game

1. Copy `winhttp.dll` and `winhttp_orig.dll` to your game directory
2. Copy the `mods` folder to your game directory
3. Place mod DLLs in the `mods` folder
4. Launch the game