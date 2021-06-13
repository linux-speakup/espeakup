---
date: 5 Nov 2008
section: 8
title: ESPEAKUP
---

# NAME

espeakup --- connect Speakup to the ESpeak TTS engine

# SYNOPSIS

**espeakup** \[ **- -pid-path=path** \] \[
**- -default-voice=voicename** \] \[ **- -debug** \] \[ **- -help** \]
\[ **- -version** \]

# OPTIONS

**-P path, - -pid-path=path**

:   Set the full path for the pid file espeakup uses when in daemon
    mode.

**-V voicename, - -default-voice=voicename**

:   Set the espeak voice to be used by default.

**-d, - -debug**

:   run in the foreground, rather than becoming a daemon process.

**-h, - -help**

:   display a brief help message and exit.

**-v, - -version**

:   output version information and exit.

# DESCRIPTION

Espeakup bridges the gap between two tools: the Speakup screen review
system and the ESppeak text-to-speech engine. Each of these tools
performs a well-defined task. Speakup is a kernel-based screen reader
for the Linux console. It extracts and processes the text that is
displayed on the foreground virtual console. It supports several
hardware based speech synthesizers directly. However, since it is in
kernel space, it cannot support a software speech synthesizer directly
since these are in user space. ESpeak is a popular software speech
synthesizer. It is small, light weight, very responsive, and supports
multiple languages. Espeakup is a connector which will read text sent to
it by speakup and forward it to ESpeak. This allows Speakup to use
ESpeak as its speech synthesizer.

Espeakup is a daemon. Typically, it is started at boot time, and it
terminates when the system is halted or rebooted. It should be started
by the system\'s init scripts. This process varies among Linux
distributions, but the details are usually managed by the person who
packaged Espeakup for your distribution. From the perspective of an
average user, Espeakup\'s operation is invisible.

# BUGS

Espeakup is still classified as alpha software. Bugs are periodically
found and fixed. If you find a bug, please do report it to the author.
You might also consider mentioning it on the mailing list for the
Speakup screenreader. Visit
http://speech.braille.uwo.ca/mailman/listinfo/speakup to learn more
about the mailing list.

# SEE ALSO

For more information about Speakup, visit its homepage:
http://linux-speakup.org. ESpeak\'s home page is
http://espeak.sourceforge.net.

# AUTHOR

William Hubbs is the author and maintainer of Espeakup. He may be
reached via the email address \<w.d.hubbs\@gmail.com>. This manual page
was written by Chris Brannon, and his email address is
\<cmbrannon79\@gmail.com>.
