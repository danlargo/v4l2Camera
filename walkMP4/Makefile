CC=gcc
CXX=g++
RM=rm -f
RRM=rm -rf
MD=mkdir -p
CP=cp

CPPFLAGS=-g -std=c++20
LDLIBS=
LDFLAGS=-g

# Source files
SRCS := $(wildcard *.cpp)

# Object files
OBJS := $(patsubst %.cpp, build/%.o, $(SRCS))

# Default target
all: ../bin/walkMP4 dictionary

# Ensure the build directory exists
$(shell $(MD) build)

# Link the target executable
../bin/walkMP4: $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# Pattern rule to compile .cpp files to .o files
build/%.o: %.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@

dictionary: walkMP4_dictionary.json
	$(CP) walkMP4_dictionary.json ../bin/walkMP4_dictionary.json

# Clean target
clean:
	$(RM) ../bin/walkMP4 ../bin/walkMP4_dictionary.json $(OBJS)
	$(RRM) build

.PHONY: all clean

