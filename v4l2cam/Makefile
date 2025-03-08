CC=gcc
CXX=g++
RM=rm -f
RRM=rm -rf
MD=mkdir -p

CPPFLAGS=-g -std=c++20 -I ../distribution

# Detect architecture
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

# Set architecture-dependent library
ifeq ($(UNAME_S),Linux)
    ifeq ($(UNAME_M),x86_64)
        LDLIBS=-L../distribution -lv4l2camera-linux-amd64
    endif
    ifeq ($(UNAME_M),aarch64)
        LDLIBS=-L../distribution -lv4l2camera-linux-aarch64
    endif
endif

LDFLAGS=-g

# Source files
SRCS := $(wildcard *.cpp) $(wildcard image_utils/*.cpp)

# Object files
OBJS := $(patsubst %.cpp, build/%.o, $(SRCS))
OBJS := $(patsubst image_utils/%.cpp, build/image_utils/%.o, $(OBJS))

# Default target
all: ../bin/v4l2cam

# Ensure the build directory exists
$(shell $(MD) build)
$(shell $(MD) build/image_utils)
$(shell $(MD) ../bin)

# Link the target executable
../bin/v4l2cam: $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# Pattern rule to compile .cpp files to .o files
build/%.o: %.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@

build/image_utils/%.o: image_utils/%.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@

# Clean target
clean:
	$(RM) ../bin/v4l2cam $(OBJS)
	$(RRM) build

.PHONY: all clean

