CC=gcc
CXX=g++
RM=rm -f

CPPFLAGS=-g -std=c++20
LDLIBS=
LDFLAGS=-g

# Source files
SRCS := $(wildcard *.cpp)

# Object files
OBJS := $(SRCS:.cpp=.o)

# Target executable
TARGET := decodeMP4

# Default target
all: $(TARGET)

# Link the target executable
$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# Pattern rule to compile .cpp files to .o files
%.o: %.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@

# Clean target
clean:
	$(RM) $(TARGET) $(OBJS)

.PHONY: all clean

