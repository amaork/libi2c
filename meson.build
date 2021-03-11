project('i2c', ['c'],
  version: run_command(
    'head', '-n', '1', files('VERSION'),
  ).stdout().strip(),
  meson_version: '>= 0.56.0',
  license: 'MIT',
  default_options: [
    'c_std=c11',
    'warning_level=2',  # -Wall, -Wextra
    'werror=true',  # -Werror
  ],
)

# the c++ compiler object
cc = meson.get_compiler('c')

# global cflags (public, included in subprojects and pkg-config)
cflags = ['-DLIBI2C_VERSION="' + meson.project_version() + '"']

# per compiler cflags, if desired
if cc.get_id() == 'clang'
  cflags += []
else
  cflags += []
endif

# split the project version
proj_ver = meson.project_version().split('.')
# version minus patch revision
proj_ver_short = proj_ver[0] + '.' + proj_ver[1]
proj_ver_major = proj_ver[0]
proj_ver_minor = proj_ver[1]
proj_ver_patch = proj_ver[2]

# all of this is used by configure_file on i2c_config.h.in
proj_description = 'Linux userspace i2c operation library'
proj_url = 'https://github.com/amaork/libi2c'
binary_package = 'libi2c'
origin = 'github'

subdir('include')
subdir('src')
subdir('example')
subdir('tests')