project('chip8_cc', ['c', 'cpp'],
  version : '0.1',
  default_options : [
    'warning_level=3',
    'cpp_std=c++17',
    'buildtype=debug'
  ]
)

cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')

dependencies = []

if host_machine.system() == 'windows'
 add_project_link_arguments(
    '/FORCE:MULTIPLE',
    language: ['c', 'cpp']
  )
endif


# Raylib configuration
cmake = import('cmake')
raylib_opts = cmake.subproject_options()
raylib_opts.set_install(false)
raylib_opts.add_cmake_defines({
  'BUILD_EXAMPLES': 'OFF'
  #'PLATFORM': 'Desktop'
})

raylib_subproject = cmake.subproject('raylib', options: raylib_opts)
dependencies += raylib_subproject.dependency('raylib')


# System-specific dependencies
if host_machine.system() == 'windows'
  dependencies += cc.find_library('winmm')
elif host_machine.system() == 'linux'
  dependencies += [
    cc.find_library('m'),
    cc.find_library('dl')
  ]
endif

# Source files
sources = [
  'src/main.cpp',
  'src/chip8.cpp'
]

# Include directories
include_dirs = include_directories('include')

# Create executable
executable('chip8_cc',
  sources,
  include_directories : include_dirs,
  dependencies : dependencies,
  install : true
)

# Unit tests
if get_option('buildtype').contains('debug')
  test_sources = [
    'tests/chip8_tests.cpp',
    'src/chip8.cpp'
  ]
  
  test_exe = executable('chip8_tests',
    test_sources,
    include_directories : include_dirs,
    dependencies : dependencies
  )
  
  test('chip8_tests', test_exe)
endif