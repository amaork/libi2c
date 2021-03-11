# source for core library
libi2c_src = [
  'i2c.c',
]

# shared and/or static library
libi2c = library(meson.project_name(), libi2c_src,
  c_args: [
    cflags,
    '-D_DEFAULT_SOURCE',
  ],
  install: true,
)

# dependency for use in subprojects (only)
libi2c_dep = declare_dependency(
  compile_args: cflags,
  include_directories: include_directories('.'),
  link_with: libi2c,
  version: meson.project_version(),
)

# pkg-config support
pkg = import('pkgconfig')
pkg.generate(libi2c,
  extra_cflags: cflags,
  subdirs: meson.project_name(),
)

# public headers
install_headers(['i2c.h'],
  # i2c.h can clash with linux/i2c.h, so we put it in a subdir
  subdir: meson.project_name(),
)

# python module build follows
pymod = import('python')
python_versions = ['python2', 'python3']
python_testme = []  # python execs and built libraries go in here for testing
python_src = ['pyi2c.c']
python_path = meson.current_build_dir()  # used for tests

foreach python_version : python_versions
  python = pymod.find_installation(python_version, required: false)
  if python.found()
    # we still need the python-dev package
    python_dep = dependency(python_version)
    message(python_version + ' module enabled')
    # same arguments as `shared_module`, more or less
    # https://mesonbuild.com/Python-module.html
    pylib = python.extension_module('pylibi2c', python_src,
      dependencies: python_dep,
      link_with: libi2c,
      install: true,
      c_args: cflags,
    )
    python_testme += {'python': python, 'lib': pylib}
  endif
endforeach