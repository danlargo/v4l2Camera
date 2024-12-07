<hr/>

# Overview

V4l2Camera is an attempt to abstract the video4linux2 api into something more easily usable in a generic C++ application
- I set out, naively, to create cross-platorm support (Linux, MacOS, Windows) but soon realized the extent of that effort
- The linux version is "done", although I know there are lots of refinements possible/needed/required
- I think I see a path forward on the MacOS and Windows side of things after starting down a few interesting pathways (libuvc, libusb)

<br/>

## Details
- Low level camera control using (or emulating) the [video4linux2](https://www.kernel.org/doc/html/v6.12/userspace-api/media/v4l/v4l2.html) interface
- Currently supporting Linux based systems (tested on Debian AMD64 and Raspbian ARM64 platforms)
    - support planned for MacOS (via AVFoundation api), and
    - Windows (via Media.Capture api)

- Designed to support both command line and GUI based applications
- Example code provided for command line application, sample application (binary-only) provided for GUI based application
- Distributed as open-source under the [MIT License](./LICENSE)


<br/><br/><hr/>

# Index
- [-here-](#distribution) V4l2Camera Overview                       
- [-here-](./API_Readme.md) API/Class Details                         
- [-here-](./example/ExampleCode_Readme.md) Command Line Example Code                 
- [-here-](./camControl/camControl_Readme.md)camControl Cross Platform GUI app         
- [-here-](./image_conversion_functions/ImageConversion_Readme.md) Image Conversion Functions                


<br/><br/><hr/>

# Distribution

- The v4l2camera class source code is distributed as open source under the [MIT License](./LICENSE)
    - If you use it and find it useful, drop me a line and let me know
    - Provide attribution and a link back to the github repo so more people can find the code and use it (please) 
    - I have gained so much information from the open source community that I figured it was time to give back
- The [camControl](./camControl/camControl_Readme.md) app is distributed free of charge on Linux platforms, all rights for this app remain with the developer
    - will decide how to distribute the MacOS and Windows versions once they exist


<br/><br/><hr/>

# Build Dependencies

### General
- Makefile is absolutely generic, I spent time on the source code and know I could clean up the Makeifle, will get to it :)
    - Currenly uses the gcc/g++ compiler and has been tested on Linux and MacOS (sort of)
    - MacOS version has not attempted to include AVFoundation api so may require changes to use the LLVM compiler and different include paths
    - Windows version has not been started
- Makefile will be updated to use native compiler on each platform (as required, would prefer to use gcc/g+ if possible)

### Linux
- Dependent only on the [std](https://en.cppreference.com/w/cpp/standard_library) C++ library

### MacOS
- In progress, will (hopefully) be designed to work with the [AVFoundation](https://developer.apple.com/av-foundation/) api

### Windows
- In progress, will besigned to work with the [Media.Capture](https://learn.microsoft.com/en-us/uwp/api/windows.media.capture.mediacapture?view=winrt-26100) api



<br/><br/><hr/>

# Building V4l2Camera into your own App

### Download

- Download or clone the git repo (this repo)
```
$ git clone https://github.com/danlargo/v4l2Camera.git

```

### Make

- Run **make** in the source folder, this will create the example binary (v4l2cam) and create a distribution folder for inclusion in other apps (v4l2cam-dist)
```
$ cd v4l2camera
$ make

```
- Copy the distribution folder into the top level of your project
```
$ cp -r ./v4l2cam-dist ../myProject

```

### Add to your project source - Linux

```
#include "../linux/linuxcamera.h"
.
.
.
// generate a list of UVC cameras available in the system

std::vector<LinuxCamera*> camlist = LinuxCamera::DiscoverCameras();

if( camlist.size() == 0 ) std::cerr << "No Cameras in System??" << std::endl;
else
{
    LinuxCamera * cam = camList[0];
    if( !cam->open() ) std::cerr << "Camera is not accessible - Not Open" << std::endl;
    else
    {
        // do stuff here

    }
}

```

### Add to your Makefile
```
g++ -g -std=c++20 -o main.o main.c
g++ -g -o main main.o v4l2cam-dist/libv4l2cam-linux-arm64.a


```


<br/><br/><hr/>

# Example Code

- See command line example [folder](./example/ExampleCode_Readme.md)



<br/><br/><hr/>

# Sample App - camControl

- [camControl](./camControl/camControl_Readme.md) is built to run in Linux AMD64 and ARM64 GUI environments
    - built on Debian Linux (AMD64), and
    - Raspbian, ARM64
- It is distributed on Linux platforms free of charge.


<br/><br/><hr/>

# Class method documentation

- [Here](./API_Readme.md)


<br/><br/><hr/>

# Image Conversion Functions

- Information I have collected around the internet regarding conversion from YUV imager formats to generic RGB32 bitmaps
- [Here](./image_conversion_functions/ImageConversion_Readme.md)


<br/><br/><hr/>

# ToDo

- Add better support for fourCC codes in Video modes
    - currently lookup and display is managed via string version of fourCC code, which is not guarranteed to be unique
    - fourCC convertion functions are provided in v4l2camera.cpp but not used in video mode queries and selection

- Add User Control interface into example command line app (v4l2cam)
    - User Controls are fully supported in the V4l2Camera class

- MacOS support for V4l2Camera
    - went down a few blind alleys...
        - libucv [here](https://libuvc.github.io/libuvc/)
            - not unhappy with the api, but found it frustrating in that they still haven't dealt with the app permission issue on MacOS (which may not be solveable)
        - libusb [here](https://libusb.info/)
            - for obvious reasons it felt like I was simply duplicating the libuvc effort
        - QCamera (on Qt 6) [here](https://doc.qt.io/qt-6/qcamera.html)
            - this effort proved the most frustrsating as I am using Qt as a cross platform GUI development environment (maybe that is my issue)
            - Qt 6 is not supported on Raspbian, which is where the majority of my development is these days, Qt 5 does not support QCamera
            - QCamera is able to fetch images from USB cameras on MacOS but has zero support for any User Controls
                - I have a question ticket [here](https://forum.qt.io/topic/159507/qcamera-and-supportedfeatures-does-this-work-on-macos?lang=en-US) into Qt Forum but have received no response
                - This seems to be an issue that has existed for a while.
    - Everything I am reading is pointing towards having to deal with the AVFoundation api from Apple.
        - All documentation points to XCode and Swift/Objective-C
        - I want to provide command line and GUI support for C++, so may be banging my head on a wall here
        - I want to make sure to properly support the request and maintenance of proper user/device permissionsas well

- MacOS build of camControl
    - will use this an indication that the class library actually works on MacOS

- Windows support
    - looking at the Media.Capture api
    - very preliminary

- Windows build of camControl
    - will use this an indication that the class library actually works on Windows
