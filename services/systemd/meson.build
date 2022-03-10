unitdir = systemd.get_variable(pkgconfig: 'systemdsystemunitdir')
prefixdir = get_option('prefix')
bindir = join_paths(prefixdir, get_option('bindir'))

unit_conf = configuration_data()
unit_conf.set('bindir', bindir)

service_file = configure_file(
  input : 'espeakup.service.in',
  output : 'espeakup.service',
  configuration : unit_conf
)

install_data(service_file,
  install_dir : unitdir
)
