# v4l2Camera

Low level camera control using (or simulating) the [video4linux2](https://www.kernel.org/doc/html/v6.12/userspace-api/media/v4l/v4l2.html) interface.

Currently supporting Linux based systems (tested on Debian AMD64 and Raspbian ARM64 platforms).

Compiles into both command line and GUI based systems.

Support for MACOS and Windows in progress.



## Distribution

v4l2Camera is distributed as open source under the [MIT License](./LICENSE)

camControl app is distributed free of charge on Linux platforms, all rights for this app remain with the developer.



## Development Dependencies

Linux : Dependent only on the std C++ namespace support.

MACOS : Designed to work with the AVFoundation API (coming soon)

WINDOWS : Designed to work with the 


## Building

Download or clone the git repo.

Run <make> in the source folder, this will create the example binary (v4l2cam) and create a distribution folder for inclusion in other apps (v4l2cam-dist).

Copy the distribution folder into the top level of your project.

### Linux

Develop...

```
#include "../linux/linuxcamera.h"

// generate a list of UVC cameras available in the system

std::vector<LinuxCamera*> camlist = LinuxCamera::DiscoverCameras();


```

Make...

- statically link against <v4l2cam-dist/libv4l2cam-linux-arm64.a>
- see examples in [Makefile](./Makefile)


## Distribution Files

Linux : requires only libv4l2cam-linux-<arch>.a and v4l2camera.h

MACOS : tbd

WINDOWS : tbd



## Example Code

See command line example [folder](./example/README.md)



## Sample App

[camControl](./camApp-Install/README.md) is built to run in Linux AMD64 and ARM64 GUI environments. It is distributed on Linux platforms free of charge.


## API Dcoumentatiom

Jump [here](./V4l2Camera_API.md)