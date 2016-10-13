#!/bin/sh
gcc instrument.c -g -fPIC -lzlog -shared -o libinstrument.so
sudo cp libinstrument.so /usr/lib64/
#sudo cp libinstrument.so /usr/lib/x86_64-linux-gnu/
