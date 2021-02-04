project('espeakup', 'c',
  version : '0.81',
  license : 'GPL-3.0-or-later',
  default_options : ['buildtype=debugoptimized', 'c_std=gnu11', 'warning_level=3'])

cc = meson.get_compiler('c')
thread_dep = dependency('threads')
espeak_dep = dependency('espeak-ng')
alsa_dep = dependency('alsa')
math_dep = cc.find_library('m', required : false)

subdir('src')
subdir('doc')
subdir('services')

executable('espeakup',
           espeakup_sources,
           espeakup_version,
           include_directories : espeakup_incdir,
           dependencies : [thread_dep, espeak_dep, alsa_dep, math_dep],
           install : true)
