#
# NOTE TO DEVELOPER
#
# This file is named GNUmakefile to ensure that GNU make command, used on Linux and MacOS, is used to build the project.
#
#   - this allows the Window NMAKE command to default to the Makefile.
#	- unfortuantely, the Window NMAKE and MAKE can not co-exist in the same file without a bunch of really weird hacks
#
#
# 
CC=gcc
CXX=g++
RM=rm -f
RRM=rm -rf
MD=mkdir

#
# Linux and architecture specific compile and link flags
#
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

ifeq ($(UNAME_S),Linux)

	CPPFLAGS=-g -std=c++20 -I v4l2camera-dist
	LDLIBS=
	LDFLAGS=-g 

	ifeq ($(UNAME_M),x86_64)
		STATIC_LIBS=v4l2camera-dist/libv4l2camera-linux-amd64.a
	endif

	ifeq ($(UNAME_M),aarch64)
		STATIC_LIBS=v4l2camera-dist/libv4l2camera-linux-aarch64.a
	endif

else 

#
# MACOS specific compile and link flags
#	
	ifeq ($(UNAME_S),Darwin)
		CC=clang
		CXX=clang++ 
		CPPFLAGS=-g -std=c++20 -arch arm64 -mmacosx-version-min=15.0 -I v4l2camera-dist
		LDLIBS=-framework AVFoundation -framework Foundation -framework CoreMedia
		LDFLAGS=-g
		STATIC_LIBS=v4l2camera-dist/libv4l2camera-macos.a
	endif

endif


#
# V4l2 build targets
#
all: dist v4l2cam

#
# Include Files
#
v4l2camera.h: source/v4l2camera.h
	cp source/v4l2camera.h v4l2camera-dist

linuxcamera.h: source/linuxcamera.h
	cp source/linuxcamera.h v4l2camera-dist

maccamera.h: source/maccamera.h
	cp source/maccamera.h v4l2camera-dist

v4l2cam_defs.h: source/v4l2cam_defs.h
	cp source/v4l2cam_defs.h v4l2camera-dist

objccamera.h: source/objccamera.h
	cp source/objccamera.h v4l2camera-dist
	
i_objccamera.h: source/i_objccamera.h
	cp source/i_objccamera.h v4l2camera-dist

wincamera.h: source/wincamera.h
	cp source/wincamera.h v4l2camera-dist

#
#
# Distribution Files
#
v4l2camera.o: source/v4l2camera.cpp v4l2camera.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o build/v4l2camera.o -c source/v4l2camera.cpp

linuxcamera.o: source/linuxcamera.cpp v4l2camera.h linuxcamera.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o build/linuxcamera.o -c source/linuxcamera.cpp 

maccamera.o: source/maccamera.cpp v4l2camera.h maccamera.h v4l2cam_defs.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o build/maccamera.o -c source/maccamera.cpp 

objccamera.o: source/objccamera.mm maccamera.h v4l2camera.h objccamera.h v4l2cam_defs.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o build/objccamera.o -c source/objccamera.mm 
	
i_objccamera.o: source/i_objccamera.mm v4l2camera.h maccamera.h i_objccamera.h i_objccamera.h v4l2cam_defs.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o build/i_objccamera.o -c source/i_objccamera.mm 


#
# Linux dist target
#
ifeq ($(UNAME_S),Linux)

ifeq ($(UNAME_M),x86_64)

dist: linuxcamera.o v4l2camera.o
	ar rcs build/libv4l2camera-linux-amd64.a build/linuxcamera.o build/v4l2camera.o
	mv build/libv4l2camera-linux-amd64.a v4l2camera-dist
	sha256sum v4l2camera-dist/libv4l2camera-linux-amd64.a > v4l2camera-dist/libv4l2camera-linux-amd64.sha256sum
endif

ifeq ($(UNAME_M),aarch64)

dist: linuxcamera.o v4l2camera.o
	ar rcs build/libv4l2camera-linux-aarch64.a build/linuxcamera.o build/v4l2camera.o
	mv build/libv4l2camera-linux-aarch64.a v4l2camera-dist
	sha256sum v4l2camera-dist/libv4l2camera-linux-aarch64.a > v4l2camera-dist/libv4l2camera-linux-aarch64.sha256sum
endif

else

#
# MACOS dist target
#
ifeq ($(UNAME_S),Darwin)

dist: maccamera.o objccamera.o v4l2camera.o i_objccamera.o
	ar rcs build/libv4l2camera-macos.a build/maccamera.o build/v4l2camera.o build/objccamera.o build/i_objccamera.o
	mv build/libv4l2camera-macos.a v4l2camera-dist
	sha256sum v4l2camera-dist/libv4l2camera-macos.a > v4l2camera-dist/libv4l2camera-macos.sha256sum

endif

endif

#
#
# v4l2cam command line tool targets
#
v4l2cam: dist main.o utils.o print.o list.o capture.o control.o fromYUV.o greyScale.o interleavedYUV420.o planarYUV420.o saveRGB24ToBMP.o yuv422.o
	$(CXX) $(LDFLAGS) -o v4l2cam build/main.o build/utils.o build/print.o build/list.o build/capture.o build/control.o \
							build/fromYUV.o build/greyScale.o build/interleavedYUV420.o build/planarYUV420.o build/saveRGB24ToBMP.o build/yuv422.o  \
							$(LDLIBS) $(STATIC_LIBS)

includes: v4l2cam-src/defines.h v4l2camera.h linuxcamera.h maccamera.h wincamera.h v4l2cam_defs.h objccamera.h i_objccamera.h

main.o: includes v4l2cam-src/main.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o build/main.o -c v4l2cam-src/main.cpp

utils.o: includes v4l2cam-src/utils.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o build/utils.o -c v4l2cam-src/utils.cpp

print.o: includes v4l2cam-src/print.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o build/print.o -c v4l2cam-src/print.cpp
	
list.o: includes v4l2cam-src/list.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o build/list.o -c v4l2cam-src/list.cpp
	
capture.o: includes v4l2cam-src/capture.cpp fromYUV.o greyScale.o interleavedYUV420.o planarYUV420.o saveRGB24ToBMP.o yuv422.o
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o build/capture.o -c v4l2cam-src/capture.cpp

control.o: includes v4l2cam-src/control.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o build/control.o -c v4l2cam-src/control.cpp

#
# Image Utility Targets
#
fromYUV.o: includes v4l2cam-src/image_utils/fromYUV.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o build/fromYUV.o -c v4l2cam-src/image_utils/fromYUV.cpp

greyScale.o: includes v4l2cam-src/image_utils/greyScale.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o build/greyScale.o -c v4l2cam-src/image_utils/greyScale.cpp

interleavedYUV420.o: includes v4l2cam-src/image_utils/interleavedYUV420.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o build/interleavedYUV420.o -c v4l2cam-src/image_utils/interleavedYUV420.cpp

planarYUV420.o: includes v4l2cam-src/image_utils/planarYUV420.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o build/planarYUV420.o -c v4l2cam-src/image_utils/planarYUV420.cpp

saveRGB24ToBMP.o: includes v4l2cam-src/image_utils/saveRGB24ToBMP.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o build/saveRGB24ToBMP.o -c v4l2cam-src/image_utils/saveRGB24ToBMP.cpp

yuv422.o: includes v4l2cam-src/image_utils/yuv422.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o build/yuv422.o -c v4l2cam-src/image_utils/yuv422.cpp


#
# Clean before build
#
clean:
	$(RM) v4l2cam
	$(RRM) build
	$(MD) build
	${RM} v4l2camera-dist/*.h

#
# Clean Targets
#
#
# Linux
ifeq ($(UNAME_S),Linux)

#
# AMD64 Target
ifeq ($(UNAME_M),x86_64)
	$(RM) v4l2camera-dist/*-linux-amd64.a
	$(RM) v4l2camera-dist/*-linux-amd64.sha256sum
endif

#
# ARM64 Target
ifeq ($(UNAME_M),aarch64)
	$(RM) v4l2camera-dist/*-linux-aarch64.a
	$(RM) v4l2camera-dist/*-linux-aarch64.sha256sum
endif

# ---- not Linunx
else

#
# MacOS
ifeq ($(UNAME_S),Darwin)
	$(RM) v4l2camera-dist/*-macos.a
	$(RM) v4l2camera-dist/*-macos.sha256sum

endif

endif
