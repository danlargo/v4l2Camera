CC=gcc
CXX=g++
RM=rm -f
RRM=rm -rf
MD=mkdir

UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)
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
		STATIC_LIBS=v4l2cam-dist/libv4l2cam-macos.a
	endif

#
# Linux and architecture specific compile and link flags
#
	ifeq ($(UNAME_S),Linux)

		CPPFLAGS=-g -std=c++20
		LDLIBS=

		ifeq ($(UNAME_M),x86_64)
			STATIC_LIBS=v4l2cam-dist/libv4l2cam-linux-arm64.a
		endif

		ifeq ($(UNAME_M),aarch64)
			STATIC_LIBS=v4l2cam-dist/libv4l2cam-linux-aarch64.a
		endif

	endif
endif

LDFLAGS=-g 

#
# V4l2 build targets
#
all: example dist

#
#
# Distribution Files
#
v4l2camera.o: v4l2camera.cpp v4l2camera.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o build/v4l2camera.o -c v4l2camera.cpp

linuxcamera.o: linux/linuxcamera.cpp linux/linuxcamera.h v4l2camera.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o build/linuxcamera.o -c linux/linuxcamera.cpp 

maccamera.o: macos/maccamera.cpp macos/maccamera.h v4l2camera.h macos/v4l2cam_defs.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o build/maccamera.o -c macos/maccamera.cpp 

ifeq ($(UNAME_S),Linux)

ifeq ($(UNAME_M),x86_64)
dist: linuxcamera.o v4l2camera.o
	ar rcs build/libv4l2cam-linux-arm64.a build/linuxcamera.o build/v4l2camera.o
	mv build/libv4l2cam-linux-arm64.a v4l2cam-dist
endif

ifeq ($(UNAME_M),aarch64)
dist: linuxcamera.o v4l2camera.o
	ar rcs build/libv4l2cam-linux-aarch64.a build/linuxcamera.o build/v4l2camera.o
	mv build/libv4l2cam-linux-aarch64.a v4l2cam-dist
endif

	cp linux/linuxcamera.h v4l2cam-dist/linux
	cp v4l2camera.h v4l2cam-dist

endif

ifeq ($(UNAME_S),Darwin)

dist: maccamera.o v4l2camera.o
	ar rcs build/libv4l2cam-macos.a build/maccamera.o build/v4l2camera.o
	mv build/libv4l2cam-macos.a v4l2cam-dist
	cp macos/maccamera.h v4l2cam-dist/macos
	cp v4l2camera.h v4l2cam-dist

endif

#
#
# Common Example targets
#
example: dist main.o utils.o print.o list.o capture.o
	$(CXX) $(LDFLAGS) -o v4l2cam build/main.o build/utils.o build/print.o build/list.o build/capture.o $(LDLIBS) $(STATIC_LIBS)

includes: example/defines.h v4l2camera.h linux/linuxcamera.h macos/maccamera.h macos/v4l2cam_defs.h

main.o: includes example/main.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o build/main.o -c example/main.cpp
	
utils.o: includes example/utils.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o build/utils.o -c example/utils.cpp

print.o: includes example/print.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o build/print.o -c example/print.cpp
	
list.o: includes example/list.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o build/list.o -c example/list.cpp
	
capture.o: includes example/capture.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o build/capture.o -c example/capture.cpp

#
# Clean before build
#
clean:
	$(RM) v4l2cam
	$(RRM) build
	$(MD) build

ifeq ($(UNAME_S),Linux)

ifeq ($(UNAME_M),x86_64)
	$(RM) v4l2cam-dist/*-linux-arm64.a
endif

ifeq ($(UNAME_M),aarch64)
	$(RM) v4l2cam-dist/*-linux-aarch64.a
endif

	$(RM) v4l2cam-dist/linux/linuxcamera.h
	$(RM) v4l2cam-dist/v4l2camera.h
endif

ifeq ($(UNAME_S),Darwin)
	$(RM) v4l2cam-dist/*-linux-macos.a
	$(RM) v4l2cam-dist/macos/maccamera.h
	$(RM) v4l2cam-dist/v4l2camera.h
endif