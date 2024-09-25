# v4l2Camera

Low level camera control using v4l2 ioctl interface.

Designed to run on linux based systems. Developed on a Pi5 and an Intel LInux PC, so it is sure to compile on a Debian based system if not all Linux variants.

Developed with only basic interface requirements, should compile in most systems.

Dependent on the STD C++ namespace support.


# Project Inclusion

- only requires v4l2camera.cpp and v4l2camera.h


# Example code and test interface

- main.cpp, will provide overview of basic operation, compiles into v4l2Test

- command line options :

- $ ./v4l2cam 

Usage
   -----
   -h :            this message
   -v :            show version of the V4l2Camera sub system
   -V :            operate in verbose mode (lots of information messages from low level functions)
   -S :            run totally silent so that this app will not be chatty when used in scripts
   -x :            display some sample commands
   ---
   -i :            identify all openable devices in /dev/videoX driver space
   -l :            list all USB cameras in the /dev/videoX driver space
   ---
   -d [0..63] :    select camera /dev/video<number> for operation
   -m :            list all the video modes supported by camera -d [0..63]
   -u :            list all the user controls supported by camera -d [0..63]
   ---
   -t [0..##] :    set user control [0..??] to value specified with -k [##]
   -k [0..##] :    used user control number [0..##] for set or retrieve commands
   -r :            retrieve value from user control specified by -k [##]
   ---
   -g [0..##] :    grab an image from camera -d [0..63], using video mode <number>
   -c [0..##] :    capture video from camera -d [0..63], using video mode <number>, for time -t [0..##] seconds, default is 10 seconds
   -t [0..##] :    specify a time duration for video capture, default is 10 seconds
   -o file    :    specify filename for output, will send to stdout if not set


- sample usage

   v4l2cam - example usage
   -------------------------
   ...skipping al the obvious examples
   
   ...get video modes for a /dev/video0
   $ ./v4l2cam -m -d 0
   
   ...list user controls for a /dev/video2
   $ ./v4l2cam -u -d 0
   
   ...grab an image from /dev/video4, using image mode 2, save to <test.jpg>
   $ ./v4l2cam -g 2 -d 4 -o test.jpg
   
   ...capture video from /dev/video2, using video mode 1, stream to stdout, pipe to test.mp4
   $ ./v4l2cam -c 1 -d 2 > ./test.mp4
   
   ...get the value from /dev/video2, for user control 9963776 (brightness)
   $ ./v4l2cam -r -k 9963776 -d 2
   
   ...set the value for /dev/video2, for user control 9963776 to 25
   $ ./v4l2cam -t 25 -k 9963776 -d 2

