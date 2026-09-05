# MMBasic for MacOS (MMB4M)
#
#   make                 build for Apple Silicon, the default
#   make ARCH=intel      build the x86_64 binary, from either kind of Mac
#   make clean           throw away the objects and the binary
#   make distclean       also throw away the SDL2 build
#
# The binary lands in bin/mmbasic. Everything in between lands in build/ and can
# be deleted at any time.
#
# Requirements: the Xcode Command Line Tools (xcode-select --install) and cmake
# (brew install cmake). SDL2 is downloaded and compiled in statically by this
# file, so the result is one executable that runs on a Mac with nothing else
# installed. That download is the only reason a build needs a network.

VERSION := $(shell cat VERSION 2>/dev/null || echo unknown)

# Quiet by default. `make V=1` prints every command in full.
ifeq ($(V),1)
    Q :=
else
    Q := @
endif

# Apple Silicon by default; `make ARCH=intel` builds x86_64. docs/building.md says how.
ARCH ?= silicon

# A comment after a := assignment lands inside the value, trailing spaces and
# all, so the reasons go above the lines rather than beside them.
# Big Sur is the oldest macOS with Apple Silicon; High Sierra is the oldest we
# support at all.
ifeq ($(ARCH),silicon)
    MACH := arm64
    MACOS_MIN := 11.0
else ifeq ($(ARCH),intel)
    MACH := x86_64
    MACOS_MIN := 10.13
else
    $(error ARCH must be silicon or intel, not '$(ARCH)')
endif

BUILD := build/$(ARCH)
BIN   := bin/mmbasic

# --- SDL2 ---------------------------------------------------------------------
# Homebrew's "sdl2" formula is deliberately not used: it is sdl2-compat these
# days, the SDL2 API on top of SDL3, and it carries a higher minimum macOS
# version than we target.
SDL_VERSION := 2.32.10
SDL_DIR     := build/sdl2/$(ARCH)
SDL_PREFIX  := $(SDL_DIR)/install
SDL_CONFIG  := $(SDL_PREFIX)/bin/sdl2-config
SDL_URL     := https://github.com/libsdl-org/SDL/releases/download/release-$(SDL_VERSION)/SDL2-$(SDL_VERSION).tar.gz

# --- What to compile ----------------------------------------------------------
# Upstream's build picks the "_linux" variant of every platform-split file on any
# non-Windows platform, macOS included, so the "_windows" ones are left out here
# or the link fails on duplicate symbols. The test-only files go with them.
C_SOURCES := $(shell find src -name '*.c' \
	! -path '*/gtest/*' ! -name '*stub*' ! -name 'keybuf_get_test.c' \
	! -name '*_windows.c' | sort)

# Since v0.8 there is C++ in the tree: toojpeg_streaming.cpp is compiled,
# toojpeg.cpp is the non-streaming variant upstream leaves out too. clang++
# compiles .c files as C++ despite the extension, with stricter rules of its own,
# so the two languages are compiled apart and joined at link time.
CPP_SOURCES := $(shell find src -name '*.cpp' ! -name 'toojpeg.cpp' | sort)

OBJECTS := $(C_SOURCES:%=$(BUILD)/%.o) $(CPP_SOURCES:%=$(BUILD)/%.o)
DEPS    := $(OBJECTS:.o=.d)

# -DNDEBUG is what makes this a release build, as upstream's own Release
# configuration does: without it every assert() stays live and the interpreter
# logs at Info level, writing mmb4l.log into the working directory on every run.
COMMON_FLAGS := -arch $(MACH) -mmacosx-version-min=$(MACOS_MIN) \
                -I src -I $(SDL_PREFIX)/include/SDL2 -I $(SDL_PREFIX)/include \
                -funsigned-char -w -O2 -DNDEBUG -MMD -MP

.PHONY: all clean distclean help
.DEFAULT_GOAL := all

all: $(BIN)

$(BIN): $(OBJECTS)
	@mkdir -p $(dir $@)
	@echo "  LINK   $@"
	$(Q)clang++ -o $@ $(OBJECTS) -arch $(MACH) -mmacosx-version-min=$(MACOS_MIN) \
	    -L $(SDL_PREFIX)/lib $$($(SDL_CONFIG) --static-libs)
	@# Ad-hoc signature. Without one macOS refuses to run the file at all on
	@# Apple Silicon. It does not make the binary trusted, it makes it launchable.
	@codesign -s - --force $@
	@echo "built $@ for $(MACH), macOS $(MACOS_MIN) and later"

$(BUILD)/%.c.o: %.c | $(SDL_CONFIG)
	@mkdir -p $(dir $@)
	@echo "  CC     $<"
	$(Q)clang -c $< $(COMMON_FLAGS) -o $@

$(BUILD)/%.cpp.o: %.cpp | $(SDL_CONFIG)
	@mkdir -p $(dir $@)
	@echo "  CXX    $<"
	$(Q)clang++ -c $< $(COMMON_FLAGS) -o $@

$(SDL_CONFIG):
	@echo "=== building SDL2 $(SDL_VERSION) statically for $(MACH) ==="
	@command -v cmake > /dev/null || \
	    { echo "cmake is missing. brew install cmake" >&2; exit 1; }
	@rm -rf $(SDL_DIR) && mkdir -p $(SDL_DIR)
	$(Q)curl -sSL $(SDL_URL) | tar xz -C $(SDL_DIR) --strip-components=1
	@mkdir -p $(SDL_DIR)/cmake-build
	$(Q)cd $(SDL_DIR)/cmake-build && cmake .. \
	    -DCMAKE_OSX_ARCHITECTURES=$(MACH) \
	    -DCMAKE_OSX_DEPLOYMENT_TARGET=$(MACOS_MIN) \
	    -DCMAKE_BUILD_TYPE=Release \
	    -DCMAKE_INSTALL_PREFIX=$(abspath $(SDL_PREFIX)) \
	    -DSDL_STATIC=ON -DSDL_SHARED=OFF -DSDL_TEST=OFF > /dev/null
	$(Q)cd $(SDL_DIR)/cmake-build && cmake --build . --config Release \
	    -j $(shell sysctl -n hw.ncpu) $(if $(Q),> /dev/null,)
	$(Q)cd $(SDL_DIR)/cmake-build && cmake --install . > /dev/null
	@echo "=== SDL2 ready ==="

clean:
	rm -rf build/silicon build/intel bin

distclean: clean
	rm -rf build

help:
	@echo "make                 build for Apple Silicon (the default)"
	@echo "make ARCH=intel      build the x86_64 binary"
	@echo "make clean           objects and binary"
	@echo ""
	@echo "add V=1 to any of them to see the commands in full"
	@echo "make distclean       also the SDL2 build"

-include $(DEPS)
