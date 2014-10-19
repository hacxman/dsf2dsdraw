#!/bin/sh
./dsf2dsdraw "$1" | dsd2pcm 2 L 24 | sox -S -c 2 -r 352800 -b 24 -e signed-integer -t raw - -b 24 -r 352800 -t sox - lowpass 88200 lowpass 88200 bandreject 56000 10q lowpass 50000 | sox -p -b 24 -r 96000 -p | play -p
