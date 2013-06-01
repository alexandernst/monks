require 'mkmf'

have_library("kmod") or raise

extension_name = 'lkm'

dir_config(extension_name)

create_makefile(extension_name)