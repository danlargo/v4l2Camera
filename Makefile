CC=gcc
CXX=g++
RM=rm -f

UNAME_S := $(shell uname -s)
#
# WINDOWS specific compile and link flags
#
ifeq ($(OS),Windows_NT)
	CPPFlAGS=-g
	LDLIBS=-lvideo4windows
else

#
# MACOS specific compile and link flags
	ifeq ($(UNAME_S),Darwin)
		CPPFLAGS=-g -std=c++20
		LDLIBS=-L./macos/libuvc -luvc -L/opt/homebrew/lib -lusb-1.0
	endif
#
# Linux specific compile and link flags
#
	ifeq ($(UNAME_S),Linux)
		CPPFLAGS=-g 
		LDLIBS=
	endif
endif

LDFLAGS=-g 

#
# V4l2 build targets
#
# default target
all: main

#
# Linux specific targets
#
ifeq ($(UNAME_S),Linux)

main: main.o linuxcamera.o v4l2camera.o
	$(CXX) $(LDFLAGS) -o v4l2cam main.o linuxcamera.o v4l2camera.o $(LDLIBS)

linuxcamera.o: linux/linuxcamera.cpp linux/linuxcamera.h v4l2camera.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -c linux/linuxcamera.cpp

endif

#
# MACOS specific targets
#
ifeq ($(UNAME_S),Darwin)

main: main.o macos/maccamera.o v4l2camera.o
	$(CXX) $(LDFLAGS) -o v4l2cam main.o macos/maccamera.o v4l2camera.o $(LDLIBS)

macos/maccamera.o: macos/maccamera.cpp macos/maccamera.h v4l2camera.h
	cd macos
	$(CXX) $(CFLAGS) $(CPPFLAGS) -c macos/maccamera.cpp -o macos/maccamera.o

endif


#
# Common targets
#
main.o: main.cpp defines.h v4l2camera.h linux/linuxcamera.h macos/maccamera.h macos/v4l2_defs.h 
	$(CXX) $(CFLAGS) $(CPPFLAGS) -c main.cpp

v4l2camera.o: v4l2camera.cpp v4l2camera.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -c v4l2camera.cpp


#
# Clean before build
#
clean:
	$(RM) *.o
	$(RM) v4l2cam