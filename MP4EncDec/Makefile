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
all: decodeMP4 walkMP4

# Ensure the build directory exists
$(shell $(MD) build)

# Link the target executable
decodeMP4: $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

walkMP4: build/walkMP4.o build/walkMP4_utils.o build/walkMP4_parser.o build/walkMP4_formatter.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# Pattern rule to compile .cpp files to .o files
build/%.o: %.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@

# Clean target
clean:
	$(RM) $(TARGET) $(OBJS)
	$(RRM) build

.PHONY: all clean

