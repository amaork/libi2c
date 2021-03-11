i2c_incdir = include_directories('.')

# public headers
install_headers(['i2c/i2c.h'],
  # i2c.h can clash with linux/i2c.h, so we put it in a subdir
  subdir: meson.project_name(),
)