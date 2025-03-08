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

# Object files
OBJS := $(patsubst %.cpp, build/%.o, $(SRCS))

# Default target
all: build/$(LIB_NAME) copy

# Ensure the build directory exists
$(shell $(MD) build)

# Link the target executable
build/$(LIB_NAME): $(OBJS)
	$(AR) rcs $@ $^
	$(CHKSUM) $@ > $@.sha256sum

# Copy the static library and checksum file to the distribution folder
copy: build/$(LIB_NAME)
	$(MD) ../distribution
	$(CP) v4l2camera.h $(DIST_DIR)/
	$(CP) linuxcamera.h $(DIST_DIR)/
	$(CP) build/$(LIB_NAME) $(DIST_DIR)/
	$(CP) build/$(LIB_NAME).sha256sum $(DIST_DIR)/

# Pattern rule to compile .cpp files to .o files
build/%.o: %.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@

# Clean target
clean:
	$(RM) $(OBJS)
	$(RRM) build

.PHONY: all clean copy

