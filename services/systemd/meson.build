unitdir = systemd.get_pkgconfig_variable('systemdsystemunitdir')
prefixdir = get_option('prefix')
bindir = join_paths(prefixdir, get_option('bindir'))
sysconfdir = join_paths(prefixdir, get_option('sysconfdir'))

unit_conf = configuration_data()

unit_conf.set('bindir', bindir)
unit_conf.set('sysconfdir', sysconfdir)

service_file = configure_file(
    input : 'espeakup.service.in',
    output : 'espeakup.service',
    configuration : unit_conf
)

install_data(service_file,
    install_dir : unitdir
)

install_data('espeakup.conf',
    install_dir : join_paths(sysconfdir, 'conf.d')
)
