#
#
# NOTE TO DEVELOPER
#
# This is an NMAKE style makefile, specificc to Windows nmake, which is installed by the Visual Studio build tools.
#
#
#
# WINDOWS specific compile and link flags
#
RM=del
MV=move
CP=copy
CXX=cl

CPPFLAGS=/EHsc /std:c++20 /I v4l2camera-dist

#
# V4l2 build targets
#
all: v4l2cam

#
# Include Files
#
v4l2camera.h: source\v4l2camera.h
	$(CP) source\v4l2camera.h v4l2camera-dist

v4l2cam_defs.h: source\v4l2cam_defs.h
	$(CP) source\v4l2cam_defs.h v4l2camera-dist

wincamera.h: source\wincamera.h
	$(CP) source\wincamera.h v4l2camera-dist

linuxcamera.h: source\linuxcamera.h
	$(CP) source\linuxcamera.h v4l2camera-dist

maccamera.h: source\maccamera.h
	$(CP) source\maccamera.h v4l2camera-dist

objccamera.h: source\objccamera.h
	$(CP) source\objccamera.h v4l2camera-dist
	
i_objccamera.h: source\i_objccamera.h
	$(CP) source\i_objccamera.h v4l2camera-dist

#
#
# Main Distribution Files
#
v4l2camera.obj: source\v4l2camera.cpp v4l2camera.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) /c source\v4l2camera.cpp
	$(MV) v4l2camera.obj v4l2camera-dist\v4l2camera-win.obj

wincamera.obj: source\wincamera.cpp v4l2camera.h wincamera.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) /c source\wincamera.cpp
	$(MV) wincamera.obj v4l2camera-dist

#
#
# v4l2cam command line tool targets
#
v4l2cam: includes main.obj utils.obj print.obj list.obj capture.obj control.obj wincamera.obj v4l2camera.obj
	$(CXX) /Fev4l2cam.exe build\main.obj build\utils.obj build\print.obj build\list.obj build\capture.obj build\control.obj \
							build\fromYUV.obj build\greyScale.obj build\interleavedYUV420.obj build\planarYUV420.obj build\saveRGB24ToBMP.obj build\yuv422.obj  \
							v4l2camera-dist\v4l2camera-win.obj v4l2camera-dist\wincamera.obj

includes: v4l2cam-src/defines.h v4l2camera.h linuxcamera.h maccamera.h wincamera.h v4l2cam_defs.h objccamera.h i_objccamera.h

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

	$(RM) v4l2camera-dist\wincamera.obj -Force
	$(RM) v4l2camera-dist\v4l2camera-win.obj -Force