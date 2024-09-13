CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-g 
LDFLAGS=-g 
LDLIBS=

all: main

main: main.o v4l2camera.o
	$(CXX) $(LDFLAGS) -o v4l2cam main.o v4l2camera.o $(LDLIBS)

main.o: main.cpp defines.h v4l2camera.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -c main.cpp

v4l2camera.o: v4l2camera.cpp v4l2camera.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -c v4l2camera.cpp

clean:
	$(RM) *.o

distclean: clean
	$(RM) v4l2cam