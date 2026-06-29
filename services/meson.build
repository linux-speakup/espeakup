systemd = dependency('systemd')
if (systemd.found() and get_option('systemd').allowed()) or get_option('systemd').enabled()
    subdir('systemd')
endif
