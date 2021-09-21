project(
  '03 static_library cpp',
  'cpp',
  version : '1.0',
)

lib = static_library('prog', 'lib.cpp', cpp_args : ['-Duseless'])

executable(
  'exe',
  'main.cpp',
  link_with : lib,
)
