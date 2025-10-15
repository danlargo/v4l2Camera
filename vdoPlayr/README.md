# vdoPlayr Installation Packages

This directory contains Debian (.deb) installation packages for vdoPlayr.

## Building Packages

To build the installation package for your current architecture:

```bash
make install
```

This will:
1. Clean previous builds
2. Build a release version of vdoPlayr
3. Create a Debian package with architecture-specific naming
4. Generate a SHA256 checksum file

## Available Packages

Packages are named according to the format:
- `vdoplayr_VERSION_amd64.deb` - For AMD64/x86_64 processors
- `vdoplayr_VERSION_arm64.deb` - For ARM64 processors

Each package has an accompanying `.sha256` file for integrity verification.

## Installation

To install the package:

```bash
sudo dpkg -i vdoplayr_VERSION_ARCH.deb
```

If dependencies are missing:

```bash
sudo apt install -f
```

## Verification

To verify the package integrity:

```bash
sha256sum -c vdoplayr_VERSION_ARCH.deb.sha256
```

## Removal

To uninstall:

```bash
sudo apt remove vdoplayr
```

## Package Contents

The package installs:
- `/usr/bin/vdoPlayr` - Main executable
- `/usr/share/applications/vdoplayr.desktop` - Desktop menu entry
- `/usr/share/icons/hicolor/256x256/apps/vdoplayr.png` - Application icon
- `/usr/share/pixmaps/vdoplayr.png` - Legacy icon location

## Requirements

- GTK4
- FFmpeg libraries (libavcodec, libavformat, libavutil, libswscale)
- libturbojpeg

All dependencies are automatically installed by the package manager.
