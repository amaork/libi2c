foreach d : python_testme
  python = d['python']
  testname = 'python ' + python.language_version() + ' module'
  test(testname, python,  # testname, executable
    args: files('test_pylibi2c.py'),  # full path to test.py
    depends: d['lib'],  # the library that must be built before
    workdir: python_path,
    protocol: 'exitcode',
    env: [
      'PYTHONPATH=' + python_path,
    ],
    is_parallel: false,
  )
endforeach