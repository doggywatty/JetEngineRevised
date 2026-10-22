# JetEngineRevised

A continuation of AV's/dotxr WIP open-source reimplementation of GameLoft's Jet Game Engine, along with a editor.

---

## Original Description
A reconstruction, highly buggy, remake of the engine and game code of Gameloft's *Minion Rush*
This repo contains no actual code from minion rush, everything here was reverse engineered slowly over 2 years since 2024

It is built from the game's debug symbols (a PDB for the 0.0.3 Windows test build) and checked against the disassembly of the 0.0.3 Android shared object.

## Building

The project is a C++17 codebase built with CMake (minimum 3.16). It produces static core libraries, vendored static libs, the `game` static library on top of the framework, and the `jeteditor` executable (source/editor).

### Dependencies

- Boost: download from https://www.boost.org and extract, then point CMake to it with `-DBOOST_ROOT=<path>`

### Quick build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

On Windows with Visual Studio 2022:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17 2022" -A x64 -DBOOST_ROOT=<path-to-boost>
cmake --build build --config Release -j
```

Or with Ninja + MSVC:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -G Ninja -DBOOST_ROOT=<path-to-boost>
cmake --build build -j
```

This builds the `game` library and `jeteditor` target by default.

### Tests

Tests are off by default. Enable them with `-DJET_TESTS=ON`. Some data tests require a `data/` folder (unzip `data.zip` there) and are only registered when `data/datalibs` exists:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DJET_TESTS=ON
cmake --build build -j
ctest --test-dir build
```

### Running on Windows

EGL is not available on Windows, so the editor uses **WGL** (native Windows OpenGL) instead. After building, run `jeteditor.exe` with a TrueType font:

```sh
build\jeteditor.exe --font "C:\Windows\Fonts\arial.ttf"
```

Other command-line options:
- `--world <dataDir>`: open a MinionRush data folder in edit mode
- `--select <n>`: select entity index once loaded
- `--shot <file.ppm>`: write a PPM screenshot after loading
- `--selftest <project>`: headless project check
- `--new <folder> <name>`: create a new project
- `--import <project> <file>`: import an asset

## License

The original code in this repository is licensed under the .AV Minion Rush License, Version 1.0 (see `LICENSE`).
In short: you may use it for any purpose, but if you use it in a project that is public, that project must be released as
open source with its full source code. Private projects have no such obligation.