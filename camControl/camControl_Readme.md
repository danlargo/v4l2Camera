# Overview

- This document supports the camControl application (currently executable on Debian/Ubuntu/Rapbian OSs, with AMD64 or ARM64 processors, coming soon to MacOS and Windows)
- No installer was created as there should be no dependencies on shared libraries other than Qt5 (installation/upgrade instructions provided below)
    - direct download instructions for binary for AMD64 or ARM64, sha256 checksum provided.
- UVC camera access is based on [Video4Linux](https://www.kernel.org/doc/html/v4.9/media/uapi/v4l/v4l2.html) api and the [V4l2Camera](https://github.com/danlargo/v4l2Camera) open source project


<br/><br/><hr/>

# camControl App Details

- generic UVC camera control app, providing access to all USB accessible controls and image feeds for UVC compatible cameras

![Screenshot](./camcontrol-screenshot.png)

- Can fetch and stream Motion-JPEG, YUV 422, YUV 420 and 16-bit grey-scale formats from UVC cameras (built in coversion for most formats but H.264/H.265 format)
    - if you stumble on a format that is not supported drop me a line and I will add it
- Can snapshot JPG, PNG and BMP snapshots
- Can cast Motion-JPEG video streams via HTTP multi-part streams
- Can capture AVI-MJPEG video files to file (didn't want any library dependencies on install so no MP4 capure right now)
- CamControl is based on the Qt UI api and has native support for snapshot of JPG, BMP and PNG image formats.

- [ ] To Do - add raw MJPEG, H.264 and H.265 frame capture to video support with no re-encoding (raw dump to file)
- [ ] To Do - add RTSP and RTMP casting for Motion-JPEG, H.264 and H.265 image formats.


<br/><br/><hr/>

# Installation Details

### camControl AMD64 Install
- built and tested on Ubuntu 24.04.1 LTS, Intel(R) Core(TM) i5-7600K CPU @ 3.80Ghz
- installation commands
```
cd ~/Desktop
mkdir camControl
cd camControl

wget https://github.com/danlargo/v4l2Camera/raw/refs/heads/main/camControl/camControl-amd64
wget https://github.com/danlargo/v4l2Camera/raw/refs/heads/main/camControl/camControl-amd64.sha256sum
sha256sum -c camControl-amd64.sha256sum

# if sha256sum returns OK
mv camControl-amd64 camControl
chmod +x camControl

# run it
./camControl

```

<br/><br/><hr/>

### camControl ARM64 Install - Raspberry Pi
- built and tested on Raspbian (Debian GNU/Linux 12 (bookworm)), ARM64 Cortex-A76 CPU @ 2.4Ghz (Pi5)
- installation commands
```
wget https://github.com/danlargo/v4l2Camera/raw/refs/heads/main/camControl/camControl-aarch64
wget https://github.com/danlargo/v4l2Camera/raw/refs/heads/main/camControl/camControl-aarch64.sha256sum
sha256sum -c camControl-aarch64.sha256sum

# if sha256sum returns OK
mv camControl-aarch64 camControl
chmod +x camControl

# run it
./camControl

```

<br/><br/><hr/>

### Qt5 Runtime

- camControl is developed and dependent on the Qt5 apis
> [!WARNING]
> In Debian 12 aka Bookworm, Qt5 is part of the baseline installation. 
>   - This is how the camControl installers are set up. (i.e. No Qt5 dependencies are included)
>   - If you want to install on Buster or Bullseye, run the following commands (depending on your OS) from the cmd line. 
>   - This should resolve any shared library issues with Qt5.

```
$ sudo apt-get update
$ sudo apt-get upgrade

# for Buster, Debian 10
$ sudo apt-get install qt5-default

# for Bullseye, Debian 11
$ sudo apt-get install qtbase5-dev qtchooser

```