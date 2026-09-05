[← MMB4M](../README.md)

# Building MMB4M

Binaries come with each release; this is for building it yourself.

```sh
make                 # Apple Silicon
make ARCH=intel      # x86_64, from either kind of Mac
```

The binary lands in `bin/mmbasic`. Everything in between lands in `build/` and can be deleted at any time.

## What you need

- clang, from the Xcode Command Line Tools: `xcode-select --install`
- cmake, for SDL2: `brew install cmake`
- a network connection the first time, to fetch SDL2

SDL2 is compiled in; the binary depends on nothing beyond macOS.

## Building for Intel

`make ARCH=intel` produces the x86_64 binary and works from an Apple Silicon Mac. clang cross-compiles and SDL2 is built for the same architecture alongside it. The two builds keep their objects apart, under `build/intel/` and `build/silicon/`, so switching between them does not force a full rebuild.

Both write to `bin/mmbasic`, so the last build wins. Build one, copy it somewhere, then build the other.

The Intel binary targets macOS 10.13 and later, the Apple Silicon one 11.0, which is the oldest macOS for that hardware.

## Other targets

```sh
make clean           # objects and binary
make distclean       # also the SDL2 build
make help
make V=1             # print every command in full
```

`make` prints one line per file; `V=1` prints the full compiler invocations.

## Why it is built this way

**SDL2 is built from source, not taken from Homebrew.** Homebrew's `sdl2` formula is `sdl2-compat` these days, the SDL2 API on top of SDL3, and it carries a higher minimum macOS version than this port targets. Building it here gives one self-contained executable and control over the deployment target. The build is cached under `build/sdl2/<arch>/`.

**The upstream CMakeLists.txt is not used.** It hardcodes `/usr/include/SDL2`, which is not where SDL2 is on macOS. Calling clang directly avoids rewriting somebody else's project files. cmake is still needed, for SDL2 itself.

**`*_windows.c` is left out.** The build picks the `_linux` variant of every platform-split file on any non-Windows platform, macOS included, and compiling both gives duplicate symbols at link time.

**C and C++ are compiled apart.** Since v0.8 there is C++ in the tree: `toojpeg_streaming.cpp` is compiled, `toojpeg.cpp` is the variant upstream leaves out too. clang++ compiles `.c` files as C++ despite the extension, with stricter rules of its own, so `.c` goes through `clang` and `.cpp` through `clang++`. They are joined at link time by `clang++`, which pulls in the C++ runtime.

**The binary is ad-hoc signed.** Without a signature macOS refuses to run it at all on Apple Silicon. It does not make the binary trusted. See [running MMB4M](running.md).
