#
#
# NOTE TO DEVELOPER
#
# This is an NMAKE style makefile, specificc to Windows namek, which is installed by the Visual Studio build tools.
#
#
#
# WINDOWS specific compile and link flags
#
RM=del
MV=move
CP=copy
CXX=cl

CCFLAGS = -D WIN64
!IF "$(PROCESSOR_ARCHITECTURE)" == "ARM64"
    CCFLAGS = $(CCFLAGS) -D ARM64
!ENDIF

!IF "$(PROCESSOR_ARCHITECTURE)" == "AMD64"
	CCFLAGS = $(CCFLAGS) -D AMD64
!ENDIF

CPPFLAGS=/EHsc /std:c++20 /I v4l2camera-dist

#
# V4l2 build targets
#
all: dist v4l2cam

#
# Include Files
#
v4l2camera.h: source\v4l2camera.h
	$(CP) source\v4l2camera.h v4l2camera-dist

v4l2cam_defs.h: source\v4l2cam_defs.h
	$(CP) source\v4l2cam_defs.h v4l2camera-dist

wincamera.h: source\wincamera.h
	$(CP) source\wincamera.h v4l2camera-dist

#
#
# Distribution Files
#
v4l2camera.obj: source\v4l2camera.cpp v4l2camera.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) /c source\v4l2camera.cpp
	$(MV) v4l2camera.obj build

wincamera.obj: source\wincamera.cpp v4l2camera.h wincamera.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) /c source\wincamera.cpp
	$(MV) wincamera.obj build

#
# WINDOWS dist target
#
dist: v4l2cam.lib

v4l2cam.lib: wincamera.obj v4l2camera.obj
    lib /OUT:v4l2camera-dist\v4l2camera-win64.lib build\wincamera.obj build\v4l2camera.obj
	powershell.exe -NoLogo -NoProfile -NonInteractive -ExecutionPolicy Bypass -Command "Get-FileHash v4l2camera-dist\v4l2camera-win64.lib > v4l2camera-dist\v4l2camera-win64.sha256"

#
#
# Common Example targets
#
v4l2cam: v4l2cam.lib main.obj utils.obj print.obj list.obj capture.obj control.obj
	$(CXX) /Fev4l2cam.exe build\main.obj build\utils.obj build\print.obj build\list.obj build\capture.obj build\control.obj \
							build/fromYUV.obj build/greyScale.obj build/interleavedYUV420.obj build/planarYUV420.obj build/saveRGB24ToBMP.obj build/yuv422.obj  \
							v4l2camera-dist\v4l2camera-win64.lib

includes: v4l2cam-src\defines.h v4l2camera.h wincamera.h v4l2cam_defs.h

main.obj: includes v4l2cam-src\main.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) /c v4l2cam-src\main.cpp
	$(MV) main.obj build

utils.obj: includes v4l2cam-src\utils.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) /c v4l2cam-src\utils.cpp
	$(MV) utils.obj build

print.obj: includes v4l2cam-src\print.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) /c v4l2cam-src\print.cpp
	$(MV) print.obj build
	
list.obj: includes v4l2cam-src\list.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) /c v4l2cam-src\list.cpp
	$(MV) list.obj build
	
capture.obj: includes v4l2cam-src\capture.cpp fromYUV.obj greyScale.obj interleavedYUV420.obj planarYUV420.obj saveRGB24ToBMP.obj yuv422.obj
	$(CXX) $(CFLAGS) $(CPPFLAGS) /c v4l2cam-src\capture.cpp
	$(MV) capture.obj build

control.obj: includes v4l2cam-src\control.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) /c v4l2cam-src\control.cpp
	$(MV) control.obj build


#
# Image Utility targets
#
#
fromYUV.obj: includes v4l2cam-src\image_utils\fromYUV.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) /c v4l2cam-src\image_utils\fromYUV.cpp
	$(MV) fromYUV.obj build

greyScale.obj: includes v4l2cam-src\image_utils\greyScale.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) /c v4l2cam-src\image_utils\greyScale.cpp
	$(MV) greyScale.obj build

interleavedYUV420.obj: includes v4l2cam-src\image_utils\interleavedYUV420.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) /c v4l2cam-src\image_utils\interleavedYUV420.cpp
	$(MV) interleavedYUV420.obj build

planarYUV420.obj: includes v4l2cam-src\image_utils\planarYUV420.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) /c v4l2cam-src\image_utils\planarYUV420.cpp
	$(MV) planarYUV420.obj build

saveRGB24ToBMP.obj: includes v4l2cam-src\image_utils\saveRGB24ToBMP.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) /c v4l2cam-src\image_utils\saveRGB24ToBMP.cpp
	$(MV) saveRGB24ToBMP.obj build

yuv422.obj: includes v4l2cam-src\image_utils\yuv422.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) /c v4l2cam-src\image_utils\yuv422.cpp
	$(MV) yuv422.obj build

#
# Clean before build
#
clean:
	$(RM) v4l2cam.exe -Force

	$(RM) build\*.obj -Force
	$(RM) v4l2camera-dist\*.h -Force

	$(RM) v4l2camera-dist\*-win64-amd64.a -Force
	$(RM) v4l2camera-dist\*-win64-amd64.sha256sum -Force