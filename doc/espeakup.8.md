---
title: ESPEAKUP
section: 8
date: 14 Jun 2021
---
<!-- markdownlint-disable MD036 -->

## NAME

espeakup --- connect Speakup to the espeak-ng TTS engine

## SYNOPSIS

**espeakup** \[**\--pid-path=path**\] \[**\--alsa-volume**\]
\[**\--default-voice=voicename**\] \[**\--debug**\] \[**\--help**\]
\[**\--version**\]

## OPTIONS

**-P path, \--pid-path=path**

:   Set the full path for the pid file espeakup uses when in daemon mode.

**\--alsa-volume**

:   Drive the ALSA volume. useful for live environments where volume
    adjustments maybe impossible.

**-V voicename, \--default-voice=voicename**

:   Set the espeak-ng voice to be used by default.

**-d, \--debug**

:   run in the foreground, rather than becoming a daemon process.

**-h, \--help**

:   display a brief help message and exit.

**-v, \--version**

:   output version information and exit.

## DESCRIPTION

espeakup bridges the gap between two tools: the Speakup screen review
system and the espeak-ng text-to-speech engine. Each of these tools
performs a well-defined task. Speakup is a kernel-based screen reader
for the Linux console. It extracts and processes the text that is
displayed on the foreground virtual console. It supports several
hardware based speech synthesizers directly. However, since it is in
kernel space, it cannot support a software speech synthesizer directly
since these are in user space. espeak-ng is a popular software speech
synthesizer. It is small, light weight, very responsive, and supports
multiple languages. espeakup is a connector which will read text sent to
it by speakup and forward it to espeak-ng. This allows Speakup to use
espeak-ng as its speech synthesizer.

espeakup is a daemon. Typically, it is started at boot time, and it
terminates when the system is halted or rebooted. It should be started
by the system's init scripts. This process varies among Linux
distributions, but the details are usually managed by the person who
packaged espeakup for your distribution. From the perspective of an
average user, espeakup's operation is invisible.

## BUGS

If you find a bug, please create an
[github issue](https://github.com/linux-speakup/espeakup/issues)
You might also consider mentioning it on the mailing list for the Speakup screenreader. Visit
[list page](https://linux-speakup.org/cgi-bin/mailman/listinfo/speakup)
to learn more about the mailing list.

## SEE ALSO

For more information about Speakup, visit its
[homepage](https://linux-speakup.org).

espeak-ng located at [github](https://github.com/espeak-ng/espeak-ng)

## AUTHOR

William Hubbs <w.d.hubbs@gmail.com> is the author of espeakup.
This manual page was written by Chris Brannon <cmbrannon79@gmail.com>.

current authors and maintainers can be found at
[github](https://github.com/linux-speakup/espeakup/graphs/contributors)
