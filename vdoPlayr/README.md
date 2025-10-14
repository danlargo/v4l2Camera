# vdoPlayr
Command line accessible GTK based video playback app.

## Supported Formats

vdoPlayr supports the following video container formats and codecs:

**Container Formats:**
- **AVI** (.avi) with MJPEG codec
- **MP4** (.mp4) with H.264 codec
- **MOV** (.mov) with H.264 codec - QuickTime format
- **3GP** (.3gp) with H.264 codec - 3GPP multimedia format
- **3G2** (.3g2) with H.264 codec - 3GPP2 multimedia format
- **M4V** (.m4v) with H.264 codec - iTunes video format
- **STDIN** - Raw JPEG or H.264 streams

**Decoder Support:**
- MJPEG: Hardware-accelerated decoding via libjpeg-turbo
- H.264: NVIDIA CUDA hardware-accelerated decoding via FFmpeg

## Prerequisites

vdoPlayr targets GTK 4 and FFmpeg-based decoding. Install the development packages before building:

```bash
# Ubuntu / Debian
sudo apt-get install \
	build-essential \
	libgtk-4-dev \
	libavcodec-dev \
	libavutil-dev \
	libswscale-dev
```

## Building

The Makefile supports debug and release profiles. If no target is specified the default build is `release`.

```bash
# Debug build (no optimizations, symbols enabled)
make BUILD_TYPE=debug vdoPlayr

# Release build
make BUILD_TYPE=release vdoPlayr
```

Artifacts are written to the workspace root as `vdoPlayr`. Intermediate objects live under `obj/<profile>`.

## Creating Debian Package

To create an installable Debian package:

```bash
make install
```

This will:
- Build the release version with optimizations
- Create a CPU-specific .deb package in the `install/` directory
- Generate a SHA256 checksum file
- Include desktop integration (menu entry and icons)

The package will be named: `vdoplayr_VERSION.BUILD_ARCH.deb` (e.g., `vdoplayr_1.0.17.243_amd64.deb`)

## Installing the Package

To install vdoPlayr on your system:

```bash
sudo dpkg -i install/vdoplayr_1.0.17.243_amd64.deb
```

If the installation fails due to missing dependencies, fix them with:
```bash
sudo apt-get install -f
```

After installation:
- Launch from your application menu (under "Audio & Video")
- Or run from terminal: `vdoPlayr --play [FILE]`

To remove:
```bash
sudo apt remove vdoplayr
```

## Other Install Methods

- macOS (Homebrew) – coming soon
- Flatpak – coming soon
- AppImage – coming soon

## Usage

```bash
# Display help message
./vdoPlayr --help

# Display version information
./vdoPlayr --version

# Launch the video player window (640x480, centered)
./vdoPlayr --play

# Launch the player and specify an expected frame rate for raw STDIN streams
./vdoPlayr --play --rate 30

# Launch the video player directly in full screen
./vdoPlayr --play --full
```

## Runtime parameters

| Option | Short | Description |
| --- | --- | --- |
| `--play [FILE]` | `-p` | Start playback. When a file is provided, vdoPlayr decodes that source; otherwise it expects a raw JPEG stream on STDIN. |
| `--rate <1-60>` | `-r` | Hint frame rate for STDIN streams. Ignored when playing from a file. |
| `--full` | `-f` | Begin playback in fullscreen mode. Requires `--play`. |
| `--debug` | `-d` | Mirror status updates to the console for troubleshooting. |
| `--help` | `-h` | Print command usage information. |
| `--version` | `-v` | Print version and build details. |

## Cleaning

```bash
make clean
```
