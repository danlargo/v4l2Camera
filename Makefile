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

		CPPFLAGS=-g -std=c++20 -I v4l2cam-dist
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
# Include Files
#
v4l2camera.h: src/v4l2camera.h
	cp src/v4l2camera.h v4l2cam-dist

linuxcamera.h: src/linuxcamera.h
	cp src/linuxcamera.h v4l2cam-dist

maccamera.h: src/maccamera.h
	cp src/maccamera.h v4l2cam-dist

v4l2cam_defs.h: src/v4l2cam_defs.h
	cp src/v4l2cam_defs.h v4l2cam-dist

wincamera.h: src/wincamera.h
	cp src/wincamera.h v4l2cam-dist

#
#
# Distribution Files
#
v4l2camera.o: src/v4l2camera.cpp v4l2camera.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o build/v4l2camera.o -c src/v4l2camera.cpp

linuxcamera.o: src/linuxcamera.cpp v4l2camera.h linuxcamera.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o build/linuxcamera.o -c src/linuxcamera.cpp 

maccamera.o: src/maccamera.cpp v4l2camera.h maccamera.h v4l2cam_defs.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o build/maccamera.o -c src/maccamera.cpp 

wincamera.o: src/wincamera.cpp v4l2camera.h wincamera.h
	@echo "\n\nWINDOWS - NOT YET SUPPORTED, coming soon\n" 

#
# WINDOWS dist target
#
ifeq ($(OS),Windows_NT)
dist: wincamera.o 
	@echo "Not implemented yet"

else

#
# Linux dist target
#
ifeq ($(UNAME_S),Linux)

ifeq ($(UNAME_M),x86_64)
dist: linuxcamera.o v4l2camera.o
	ar rcs build/libv4l2cam-linux-amd64.a build/linuxcamera.o build/v4l2camera.o
	mv build/libv4l2cam-linux-amd64.a v4l2cam-dist
	sha256sum v4l2cam-dist/libv4l2cam-linux-amd64.a > v4l2cam-dist/libv4l2cam-linux-amd64.sha256sum
endif

ifeq ($(UNAME_M),aarch64)
dist: linuxcamera.o v4l2camera.o
	ar rcs build/libv4l2cam-linux-aarch64.a build/linuxcamera.o build/v4l2camera.o
	mv build/libv4l2cam-linux-aarch64.a v4l2cam-dist
	sha256sum v4l2cam-dist/libv4l2cam-linux-aarch64.a > v4l2cam-dist/libv4l2cam-linux-aarch64.sha256sum
endif

else


#
# MACOS dist target
#
ifeq ($(UNAME_S),Darwin)

dist:
	@echo "Nothing to do, managed by XCode"

endif

endif

endif

#
#
# Common Example targets
#
example: dist main.o utils.o print.o list.o capture.o
	$(CXX) $(LDFLAGS) -o v4l2cam build/main.o build/utils.o build/print.o build/list.o build/capture.o $(LDLIBS) $(STATIC_LIBS)

includes: example/defines.h v4l2camera.h linuxcamera.h maccamera.h wincamera.h v4l2cam_defs.h

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
	${RM} v4l2cam-dist/*.h

#
# Clean Target
#
#
# Windows
ifeq ($(OS),Windows_NT)
	@echo "Windows - Not Implemented Yet"

# ---- not Windows
else

#
# Linux
ifeq ($(UNAME_S),Linux)

#
# AMD64 Target
ifeq ($(UNAME_M),x86_64)
	$(RM) v4l2cam-dist/*-linux-amd64.a
	$(RM) v4l2cam-dist/*-linux-amd64.sha256sum
endif

#
# ARM64 Target
ifeq ($(UNAME_M),aarch64)
	$(RM) v4l2cam-dist/*-linux-aarch64.a
	$(RM) v4l2cam-dist/*-linux-aarch64.sha256sum
endif

# ---- not Linunx
else

#
# MacOS
ifeq ($(UNAME_S),Darwin)
	@echo "Nothing to do, managed by XCode"

endif

endif

endif