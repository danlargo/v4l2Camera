CC=gcc
CXX=g++
RM=rm -f

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	CPPFLAGS=-g -I/opt/homebrew/include -std=c++20
	LDLIBS=-L/opt/homebrew/lib -lusb-1.0
else
	CPPFLAGS=-g 
	LDLIBS=
endif

LDFLAGS=-g 

all: main

ifeq ($(UNAME_S),Linux)
main: main.o uvccamera.o v4l2camera.o
	$(CXX) $(LDFLAGS) -o uvccam main.o uvccamera.o v4l2camera.o $(LDLIBS)

v4l2camera.o: v4l2camera.cpp v4l2camera.h uvccamera.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -c v4l2camera.cpp
endif

ifeq ($(UNAME_S),Darwin)
main: main.o uvccamera.o maccamera.o
	$(CXX) $(LDFLAGS) -o uvccam main.o uvccamera.o v4l2camera.o $(LDLIBS)

maccamera.o: maccamera.cpp maccamera.h uvccamera.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -c maccamera.cpp
endif

main.o: main.cpp defines.h uvccamera.h maccamera.h v4l2camera.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -c main.cpp

uvccamera.o: uvccamera.cpp uvccamera.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -c uvccamera.cpp

clean:
	$(RM) *.o
	$(RM) uvccam