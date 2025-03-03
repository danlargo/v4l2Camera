#! /bin/bash
#
# test all file variants
#
./decodeMP4 -i ./test1.mp4 | grep "????"
./decodeMP4 -i ./test2.mp4 | grep "????"
./decodeMP4 -i ./test3.mp4 | grep "????"
./decodeMP4 -i ./test4.mp4 | grep "????"
./decodeMP4 -i ./test5.mp4 | grep "????"
./decodeMP4 -i ./test6.mp4 | grep "????"
./decodeMP4 -i ./test7.mp4 | grep "????"
./decodeMP4 -i ./test8.mp4 | grep "????"
./decodeMP4 -i ./test1.mov | grep "????"
./decodeMP4 -i ./test2.mov | grep "????"
./decodeMP4 -i ./test3.mov | grep "????"

