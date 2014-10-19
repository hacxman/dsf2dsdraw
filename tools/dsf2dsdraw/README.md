This utility was intended for use with dsd2pcm https://code.google.com/p/dsd2pcm/ .
It reads Sony DSF file and writes pure multichannel DSD to standard output (console).
When used with -u parameter it provides play/pause and seek support.

This tool comes with play_dsf.sh script which allows you to play DSF files:

```bash
play_dsf.sh song.dsf
```

or

```bash
for a in *.dsf ; do play_dsf.sh "$a" ; done
```

Example usage (dsd2pcm and sox required):

Playback:

```bash
./dsf2dsdraw file.dsf | dsd2pcm 2 L 24 | sox -S -c 2 -r 352800 -b 24 -e signed-integer -t raw - -b 24 -r 352800 -t sox - lowpass 88200 lowpass 88200 bandreject 56000 10q lowpass 50000 | sox -p -b 24 -r 96000 -p | play -p
```

Export to FLAC:

```bash
./dsf2dsdraw file.dsf | dsd2pcm 2 L 24 | sox -S -c 2 -r 352800 -b 24 -e signed-integer -t raw - -b 24 -r 352800 -t sox - lowpass 88200 lowpass 88200 bandreject 56000 10q lowpass 50000 | sox -p -b 24 -r 96000 -t flac file.flac
```
