# espeakup connector

espeakup is a program which makes it possible for speakup to use
the espeak-ng software synthesizer.  It does this by reading speakup's
softsynth device and passing the text to espeak-ng which actually speaks.

## Requirements

This program works with the speakup screen reader, which can be obtained
from http://linux-speakup.org, and the
[espeak-ng](https://github.com/espeak-ng/espeak-ng) software speech
synthesizer.

You must have both of these installed and operational.  Setting them up
is beyond the scope of this document.

## Installation

The preferred way to install espeakup is using your distribution's
packaging system, but if your distribution does not have a package for
espeakup yet, espeakup just uses a Makefile, so you should be able to
change to the source directory, then type make, then as root, make
install.

## Starting Up

This program should be run after speakup is set up to communicate with a
software synthesizer and after /dev/softsynth exists.  The way this is
done is distribution specific, so it is beyond the scope of this
documentation.

## Command Line Options

Espeakup currently accepts the following command line options:

  --default-voice=voice, -V voice	Set default voice.
  --debug, -d				Debug mode (stay in the foreground).
  --help, -h				Show this help.
  --version, -v				Display the software version.

## Acknowledgements

I would like to thank Marc Mulcahy, the author of the speakup to
TTSynth connector, on which this work is based.  Also, I would like
to thank Kirk Reiser and Jonathan Duddington, the
authors of Speakup and Espeak, respectively, for their work.

## Filing Bugs

Bugs should be filed on our bug tracker at
https://github.com/linux-speakup/issues.
