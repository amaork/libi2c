examples = [
  'i2c_tools',
  'i2c_without_internal_address',
]

foreach example: examples
  exe = executable(example, example + '.c',
    link_with: libi2c,
    include_directories: i2c_incdir,
    c_args: [
      cflags,
      '-D_DEFAULT_SOURCE',
    ],
  )
endforeach