CC=gcc
CXX=g++
RM=rm -f

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	CPPFLAGS=-g -I./libuvc -std=c++20
	LDLIBS=-L./libuvc -luvc -L/opt/homebrew/lib -lusb-1.0
else
	CPPFLAGS=-g 
	LDLIBS=
endif

LDFLAGS=-g 

all: main

ifeq ($(UNAME_S),Linux)
main: main.o linuxcamera.o v4l2camera.o
	$(CXX) $(LDFLAGS) -o v4l2cam main.o linuxcamera.o v4l2camera.o $(LDLIBS)

linuxcamera.o: linuxcamera.cpp linuxcamera.h v4l2camera.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -c linuxcamera.cpp
endif

ifeq ($(UNAME_S),Darwin)
main: main.o maccamera.o v4l2camera.o v4l2_defs.o
	$(CXX) $(LDFLAGS) -o v4l2cam main.o maccamera.o v4l2camera.o v4l2_defs.o $(LDLIBS)

maccamera.o: maccamera.cpp maccamera.h v4l2camera.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -c maccamera.cpp
endif

main.o: main.cpp defines.h linuxcamera.h maccamera.h v4l2camera.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -c main.cpp

v4l2camera.o: v4l2camera.cpp v4l2camera.h v4l2_defs.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -c v4l2camera.cpp

v4l2_defs.o: v4l2_defs.cpp v4l2_defs.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -c v4l2_defs.cpp

clean:
	$(RM) *.o
	$(RM) v4l2cam