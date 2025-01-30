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
all: dist example

#
# Include Files
#
v4l2camera.h: src\v4l2camera.h
	$(CP) src\v4l2camera.h v4l2cam-dist

v4l2cam_defs.h: src\v4l2cam_defs.h
	$(CP) src\v4l2cam_defs.h v4l2cam-dist

wincamera.h: src\wincamera.h
	$(CP) src\wincamera.h v4l2cam-dist

#
#
# Distribution Files
#
v4l2camera.obj: src\v4l2camera.cpp v4l2camera.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) /c src\v4l2camera.cpp
	$(MV) v4l2camera.obj build

wincamera.obj: src\wincamera.cpp v4l2camera.h wincamera.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) /c src\wincamera.cpp
	$(MV) wincamera.obj build

#
# WINDOWS dist target
#
dist: v4l2cam.lib

v4l2cam.lib: wincamera.obj v4l2camera.obj
    lib /OUT:v4l2cam-dist\v4l2cam-win64.lib build\wincamera.obj build\v4l2camera.obj
	powershell.exe -NoLogo -NoProfile -NonInteractive -ExecutionPolicy Bypass -Command "Get-FileHash v4l2cam-dist\v4l2cam-win64.lib > v4l2cam-dist\v4l2cam-win64.sha256"

#
#
# Common Example targets
#
example: v4l2cam.lib main.obj utils.obj print.obj list.obj capture.obj control.obj
	$(CXX) /Fev4l2cam.exe build\main.obj build\utils.obj build\print.obj build\list.obj build\capture.obj build\control.obj v4l2cam-dist\v4l2cam-win64.lib

includes: example\defines.h v4l2camera.h wincamera.h v4l2cam_defs.h

main.obj: includes example\main.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) /c example\main.cpp
	$(MV) main.obj build

utils.obj: includes example\utils.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) /c example\utils.cpp
	$(MV) utils.obj build

print.obj: includes example\print.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) /c example\print.cpp
	$(MV) print.obj build
	
list.obj: includes example\list.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) /c example\list.cpp
	$(MV) list.obj build
	
capture.obj: includes example\capture.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) /c example\capture.cpp
	$(MV) capture.obj build

control.obj: includes example\control.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) /c example\control.cpp
	$(MV) control.obj build

#
# Clean before build
#
clean:
	$(RM) v4l2cam.exe -Force

	$(RM) build\*.obj -Force
	$(RM) v4l2cam-dist\*.h -Force

	$(RM) v4l2cam-dist\*-win64-amd64.a -Force
	$(RM) v4l2cam-dist\*-win64-amd64.sha256sum -Force