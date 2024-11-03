CC=gcc
CXX=g++
RM=rm -f

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	CPPFLAGS=-g -I/opt/homebrew/include
	LDLIBS=-L/opt/homebrew/lib -lusb-1.0
else
	CPPFLAGS=-g 
	LDLIBS=
endif

LDFLAGS=-g 

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