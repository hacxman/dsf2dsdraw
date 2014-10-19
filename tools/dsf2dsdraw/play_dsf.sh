#!/bin/sh
#./dsf2dsdraw "$1" | dsd2pcm 2 L 24 | sox -S -c 2 -r 352800 -b 24 -e signed-integer -t raw - -b 24 -r 352800  -p lowpass 50000 lowpass 46000 lowpass 42000  lowpass 42000 lowpass 42000 lowpass 42000 lowpass 42000 lowpass 42000 lowpass 42000 lowpass 35000 lowpass 35000 lowpass 35000 lowpass 35000 lowpass 35000 lowpass 35000 lowpass 30000 lowpass 28000 lowpass 28000 lowpass 21000 | sox -p -b 24 -r 192000 -t flac output.flac # -p | play -p
#./dsf2dsdraw "$1" | dsd2pcm 2 L 24 | sox -S -c 2 -r 352800 -b 24 -e signed-integer -t raw - -b 24 -r 352800 -t sox - lowpass 88200 lowpass 44100 lowpass 44100 lowpass 44100 lowpass 44100 | sox -p -b 24 -r 96000 -t flac output.flac # -p | play -p

# this seems ok
#./dsf2dsdraw "$1" | dsd2pcm 2 L 24 | sox -S -c 2 -r 352800 -b 24 -e signed-integer -t raw - -b 24 -r 352800 -t sox - lowpass 88200 lowpass 44100 lowpass 22000 lowpass 22000 lowpass 22000 lowpass 22000 | sox -p -b 24 -r 96000 -t flac output.flac # -p | play -p
./dsf2dsdraw "$1" | dsd2pcm 2 L 24 | sox -S -c 2 -r 352800 -b 24 -e signed-integer -t raw - -b 24 -r 352800 -t sox - lowpass 88200 lowpass 88200 bandreject 56000 10q lowpass 50000 | sox -p -b 24 -r 96000 -p | play -p

#./dsf2dsdraw "$1" | dsd2pcm 2 L 24 | sox -S -c 2 -r 352800 -b 24 -e signed-integer -t raw - -b 24 -r 192000 -t sox - lowpass 88200 lowpass 44100 lowpass 22000 lowpass 21000 lowpass 21000 | play -
