# v4l2Camera

Low level camera control using v4l2 ioctl interface.

Designed to run on linux based systems. Developed on a Pi5, so it is sure to compile on a Debian based system.

Developed with only basic interface requirements, should compile in most systems.

Dependent on the STD C++ namespace support.

# Project Inclusion

- only requires v4l2camera.cpp and v4l2camera.h


# Example code and test interface

- main.cpp, will provide overview of basic operation, compiles into v4l2Test

- command line options :

$ v4l2test -h                   # help message




$  v4l2test -l                  # list all cameras discovered in the /dev/videoX driver space (0 to 63 inclusive), only lists interaces that seem to be USB cameras





$ v4l2test -a                   # list all openable interfaces in the /dev/videoX driver space, regardless of whether they are cameras




