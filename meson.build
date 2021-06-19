project('espeakup', 'c',
  default_options : ['buildtype=debugoptimized', 'c_std=gnu11', 'warning_level=3'],
  license : 'GPL-3.0-or-later',
  version : '0.81',
  meson_version : '>=0.47.0')

cc = meson.get_compiler('c')
thread_dep = dependency('threads')
espeak_dep = dependency('espeak-ng')
alsa_dep = dependency('alsa')
math_dep = cc.find_library('m', required : false)

subdir('doc')
subdir('services')
subdir('src')

executable('espeakup',
           espeakup_version,
           espeakup_sources,
           dependencies : [thread_dep, espeak_dep, alsa_dep, math_dep],
           install : true)
