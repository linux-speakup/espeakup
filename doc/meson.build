pandoc = find_program('pandoc', required: false)

if pandoc.found()
  custom_target('espeakup-man', input:files('espeakup.8.md'), output:'espeakup.8',
    command:[pandoc, '--standalone', '--from', 'markdown', '--to', 'man',
                    '--shift-heading-level-by=-1', '--output', '@OUTPUT@',
                    '@INPUT@'],
    install: true, install_dir: join_paths(get_option('mandir'),'man8'))
else
  warning('Program "pandoc" not found!  Cannot generate man pages')
endif
