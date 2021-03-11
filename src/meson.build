libi2c_src = [
  'i2c.c',
]

libi2c = library(meson.project_name(), libi2c_src,
  c_args: cflags,
  install: true,
)

install_headers(['i2c.h'],
  subdir: 'libi2c', # i2c.h can clash with linux/i2c.h, so we put it in a subdir
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