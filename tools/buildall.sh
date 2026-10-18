#!/bin/zsh
# Build core for every target platform that has a toolchain here. Output: $1 (default ./build)
set -e
root=${0:a:h:h}; out=${1:-$root/build}
NDK=$(ls -d ~/Library/Android/sdk/ndk/* 2>/dev/null | tail -1)
b() { cmake -S $root -B $out/$1 -DCMAKE_BUILD_TYPE=Release ${@:2} >/dev/null && cmake --build $out/$1 -j8 --target core_jet core_base core_claraloader core_net core_particles >/dev/null && echo "$1 ok" }
b mac -DJET_TESTS=ON
[ -n "$NDK" ] && b android -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake -DANDROID_ABI=armeabi-v7a -DANDROID_PLATFORM=21
command -v x86_64-w64-mingw32-g++ >/dev/null && b windows -DCMAKE_SYSTEM_NAME=Windows -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++
xcrun --sdk iphoneos --show-sdk-path >/dev/null 2>&1 && b ios -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_ARCHITECTURES=arm64
# no iOS SDK: compile-check the core sources for arm64-apple-ios against the macOS SDK
xcrun --sdk iphoneos --show-sdk-path >/dev/null 2>&1 || {
  for f in $(git -C $root ls-files 'core/*.cpp'); do
    clang++ -std=c++17 -target arm64-apple-ios15 -isysroot $(xcrun --show-sdk-path) -fsyntax-only -Wno-incompatible-sysroot -Wno-#warnings \
      -I$root/core/base/include -I$root/core/jet/include -I$root/core/smath/include -I$root/core/claraloader/include -I$root/core/net/include -I$root/core/particles/include -I$root/libs/zlib/include -I$root/libs/freetype2/include -I$root/libs/pugixml10/src -I/opt/homebrew/include -DPUGIXML_NO_STL $root/$f
  done && echo "ios (syntax, macOS sdk) ok"
}
true
