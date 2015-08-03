# Make QuickTime reference movie on Mac OS X

[![Build Status](https://travis-ci.org/TalkBank/make-refmovie)](https://travis-ci.org/TalkBank/make-refmovie)

(No longer being used. Archived for historical purposes.)

## Build

Implemented in C++, uses CMake.

```
$ mkdir build
$ cd build
$ cmake ..
$ make
```

results in an executable `make-refmovie`.

## Usage 

``` console
$ make-refmovie mov|snd server path name outputDir
```

Example:

``` console
$ make-refmovie mov rtsp://stream.talkbank.org foo/bar name /tmp
```

generates a reference movie `/tmp/name.mov` containing URLs like

`rtsp://stream.talkbank.org/foo/bar/name/name_56k_S.mov`
