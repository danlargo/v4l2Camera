CC=gcc
CXX=g++
AR=ar
CHKSUM=sha256sum
RM=rm -f
RRM=rm -rf
MD=mkdir -p
CP=cp

CPPFLAGS=-g -std=c++20
LDLIBS=
LDFLAGS=-g

# Detect architecture
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

# Set architecture-dependent library name and distribution directory
DIST_DIR=../distribution

ifeq ($(UNAME_S),Linux)
    ifeq ($(UNAME_M),x86_64)
        LIB_NAME=libv4l2camera-linux-amd64.a
    endif
    ifeq ($(UNAME_M),aarch64)
        LIB_NAME=libv4l2camera-linux-aarch64.a
    endif
endif

# Source files
SRCS := $(wildcard *.cpp)
HDRS := $(wildcard *.h)

# Object files
OBJS := $(patsubst %.cpp, build/%.o, $(SRCS))

# Default target
all: copy

# Ensure the build directory exists
$(shell $(MD) build)

# Create the static library in the build directory
build/$(LIB_NAME): $(OBJS)
	$(AR) rcs $@ $(OBJS)
	$(CHKSUM) $@ > $@.sha256sum

# Copy the static library and checksum file to the distribution folder
$(DIST_DIR)/$(LIB_NAME): build/$(LIB_NAME)
	$(MD) $(DIST_DIR)
	$(CP) v4l2camera.h $(DIST_DIR)/
	$(CP) linuxcamera.h $(DIST_DIR)/
	$(CP) build/$(LIB_NAME) $(DIST_DIR)/
	$(CP) build/$(LIB_NAME).sha256sum $(DIST_DIR)/

# Convenience target
copy: $(DIST_DIR)/$(LIB_NAME)

# Pattern rule to compile .cpp files to .o files
# Compilation rule for object files (exclude v4l2camera.h from auto-dependencies to avoid cycles)
build/%.o: %.cpp linuxcamera.h
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean target
clean: version
	$(RM) build/$(LIB_NAME) build/$(LIB_NAME).sha256sum $(OBJS)
	$(RRM) build

# Manual version increment target
version:
	@bash ./updateVersion.sh v4l2camera.h
	@echo "Version updated in v4l2camera.h"

.PHONY: all clean copy version

