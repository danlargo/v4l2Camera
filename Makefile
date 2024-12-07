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

else

#
# Linux and architecture specific compile and link flags
#
	ifeq ($(UNAME_S),Linux)

		CPPFLAGS=-g -std=c++20
		LDLIBS=
		LDFLAGS=-g 

		ifeq ($(UNAME_M),x86_64)
			STATIC_LIBS=v4l2cam-dist/libv4l2cam-linux-amd64.a
		endif

		ifeq ($(UNAME_M),aarch64)
			STATIC_LIBS=v4l2cam-dist/libv4l2cam-linux-aarch64.a
		endif

	else 

#
# MACOS specific compile and link flags
#	
		ifeq ($(UNAME_S),Darwin)

		endif

	endif

endif


#
# V4l2 build targets
#
all: dist example

#
#
# Distribution Files
#
v4l2camera.o: v4l2camera.cpp v4l2camera.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o build/v4l2camera.o -c v4l2camera.cpp

linuxcamera.o: linux/linuxcamera.cpp linux/linuxcamera.h v4l2camera.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o build/linuxcamera.o -c linux/linuxcamera.cpp 

maccamera.o: macos/maccamera.cpp macos/maccamera.h v4l2camera.h macos/v4l2cam_defs.h
	@echo "\n\nMACOS - NOT YET SUPPORTED, coming soon\n" 

wincamera.o: windows/wincamera.cpp windows/wincamera.h v4l2camera.h
	@echo "\n\nWINDOWS - NOT YET SUPPORTED, coming soon\n" 

#
# WINDOWS dist target
#
ifeq ($(OS),Windows_NT)
dist: wincamera.o 

else

#
# Linux dist target
#
ifeq ($(UNAME_S),Linux)

ifeq ($(UNAME_M),x86_64)
dist: linuxcamera.o v4l2camera.o
	ar rcs build/libv4l2cam-linux-amd64.a build/linuxcamera.o build/v4l2camera.o
	mv build/libv4l2cam-linux-amd64.a v4l2cam-dist
endif

ifeq ($(UNAME_M),aarch64)
dist: linuxcamera.o v4l2camera.o
	ar rcs build/libv4l2cam-linux-aarch64.a build/linuxcamera.o build/v4l2camera.o
	mv build/libv4l2cam-linux-aarch64.a v4l2cam-dist
endif

	cp linux/linuxcamera.h v4l2cam-dist/linux
	cp v4l2camera.h v4l2cam-dist

else


#
# MACOS dist target
#
ifeq ($(UNAME_S),Darwin)

dist: maccamera.o 

endif

endif

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

ifeq ($(OS),Windows_NT)

else

ifeq ($(UNAME_S),Linux)

ifeq ($(UNAME_M),x86_64)
	$(RM) v4l2cam-dist/*-linux-amd64.a
endif

ifeq ($(UNAME_M),aarch64)
	$(RM) v4l2cam-dist/*-linux-aarch64.a
endif

	$(RM) v4l2cam-dist/linux/linuxcamera.h
	$(RM) v4l2cam-dist/v4l2camera.h
else

#
# MACOS dist target
#
ifeq ($(UNAME_S),Darwin)

endif

endif

endif