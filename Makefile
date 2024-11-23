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
		STATIC_LIBS=install/linux/libv4l2cam-linux.a
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

main: example/main.o example/utils.o example/print.o example/list.o example/capture.o install/linux/libv4l2cam-linux.a
	$(CXX) $(LDFLAGS) -o bin/v4l2cam-linux example/main.o example/utils.o example/print.o example/list.o example/capture.o $(LDLIBS) $(STATIC_LIBS)

install/linux/libv4l2cam-linux.a: linux/linuxcamera.o base/v4l2camera.o
	ar rcs libv4l2cam-linux.a linux/linuxcamera.o base/v4l2camera.o
	mv libv4l2cam-linux.a install/linux
	cp linux/linuxcamera.h install/linux
	cp base/v4l2camera.h install/linux

linux/linuxcamera.o: linux/linuxcamera.cpp linux/linuxcamera.h base/v4l2camera.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -c linux/linuxcamera.cpp -o linux/linuxcamera.o

endif

#
# MACOS specific targets
#
ifeq ($(UNAME_S),Darwin)

main: example/main.o example/utils.o macos/maccamera.o base/v4l2camera.o
	$(CXX) $(LDFLAGS) -o v4l2cam-macos example/main.o example/utils.o macos/maccamera.o base/v4l2camera.o $(LDLIBS)

libv4l2cam-linux.a: linux/linuxcamera.o base/v4l2camera.o
	ar rcs libv4l2cam-linux.a linux/linuxcamera.o base/v4l2camera.o
	cp linux/linuxcamera.h install/linux
	cp v4l2camera.h install/linux

macos/maccamera.o: macos/maccamera.cpp macos/maccamera.h base/v4l2camera.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -c macos/maccamera.cpp -o macos/maccamera.o

endif


#
# Common targets
#
example/main.o: example/main.cpp example/defines.h base/v4l2camera.h linux/linuxcamera.h macos/maccamera.h macos/v4l2_defs.h 
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o example/main.o -c example/main.cpp
	
example/utils.o: example/utils.cpp example/defines.h base/v4l2camera.h linux/linuxcamera.h macos/maccamera.h macos/v4l2_defs.h 
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o example/utils.o -c example/utils.cpp

example/print.o: example/print.cpp example/defines.h base/v4l2camera.h linux/linuxcamera.h macos/maccamera.h macos/v4l2_defs.h 
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o example/print.o -c example/print.cpp
	
example/list.o: example/list.cpp example/defines.h base/v4l2camera.h linux/linuxcamera.h macos/maccamera.h macos/v4l2_defs.h 
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o example/list.o -c example/list.cpp
	
example/capture.o: example/capture.cpp example/defines.h base/v4l2camera.h linux/linuxcamera.h macos/maccamera.h macos/v4l2_defs.h 
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o example/capture.o -c example/capture.cpp

base/v4l2camera.o: base/v4l2camera.cpp base/v4l2camera.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o base/v4l2camera.o -c base/v4l2camera.cpp


#
# Clean before build
#
clean:
	$(RM) *.o
	$(RM) example/*.o
	$(RM) linux/*.o
	$(RM) macos/*.o
	$(RM) base/*.o

ifeq ($(UNAME_S),Linux)
	$(RM) v4l2cam-linux
	$(RM) install/linux/*
	$(RM) bin/*-linux
endif

ifeq ($(UNAME_S),Darwin)
	$(RM) v4l2cam-macos
	$(RM) install/macos/*
endif
