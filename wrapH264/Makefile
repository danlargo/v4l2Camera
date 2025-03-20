CC=gcc
CXX=g++
RM=rm -f
RRM=rm -rf
MD=mkdir -p

CPPFLAGS=-g -std=c++20
LDLIBS=
LDFLAGS=-g

# Source files
SRCS := $(wildcard *.cpp)

# Object files
OBJS := $(patsubst %.cpp, build/%.o, $(SRCS))

# Default target
all: ../bin/wrapH264

# Ensure the build directory exists
$(shell $(MD) build)

# Link the target executable
../bin/wrapH264: $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# Pattern rule to compile .cpp files to .o files
build/%.o: %.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@

# Clean target
clean:
	$(RM) ../bin/wrapH264 $(OBJS)
	$(RRM) build

.PHONY: all clean

