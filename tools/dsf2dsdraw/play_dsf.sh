#!/bin/sh
dsf2dsdraw -q "$1" | dsd2pcm 2 L 24 | sox -c 2 -r 352800 -b 24 -e signed-integer -t raw - -c 2 -b 24 -r 352800 -e signed-integer -t raw - lowpass 88200 bandreject 56000 10q lowpass 50000 | sox -c 2 -b 24 -r 352800 -e signed-integer -t raw - -b 24 -r 96000 -e signed-integer -t raw - | play -q -c 2 -b 24 -r 96000 -e signed-integer -t raw -
