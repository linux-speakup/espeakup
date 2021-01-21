project('espeakup', 'c',
  version : '0.82',
  default_options : ['c_std=c11', 'warning_level=3'])

cc = meson.get_compiler('c')
thread_dep = dependency('threads')
espeak_dep = dependency('espeak-ng')
alsa_dep = dependency('alsa')
m_dep = cc.find_library('m', required : false)

executable('espeakup',
           'src/cli.c',
           'src/espeak.c',
           'src/espeakup.c',
           'src/espeakup.h',
           'src/queue.c',
           'src/queue.h',
           'src/signal.c',
           'src/softsynth.c',
           'src/stringhandling.c',
           'src/stringhandling.h',
           dependencies : [thread_dep, espeak_dep, alsa_dep, m_dep],
           install : true)
