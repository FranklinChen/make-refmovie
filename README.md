# Make QuickTime reference movie

Usage: 

``` console
$ make-refmovie mov|snd server path name outputDir
```

Example:

``` console
make-refmovie mov rtsp://stream.talkbank.org foo/bar name /tmp
```

generates a reference movie containing URLs like

`rtsp://stream.talkbank.org/foo/bar/name/name_56k_S.mov`
