project('cybercobra_compiler', 'cpp', default_options : ['cpp_std=c++23'])
program_name = meson.project_name()

incdir = [include_directories('inc')]
catch_dep = dependency('catch2-with-main')

if (meson.get_compiler('cpp').get_id() in ['g++', 'gcc']) \
    and get_option('buildtype').startswith('debug')       \
    and get_option('warning_level') == '3'
  add_project_arguments([
    '-Wshadow'
    ,'-Wformat=2'
    ,'-Wfloat-equal'
    ,'-Wconversion'
    ,'-Wlogical-op'
    ,'-Wshift-overflow=2'
    ,'-Wduplicated-cond'
    ,'-Wcast-qual'
    ,'-Wcast-align'
    ,'-D_GLIBCXX_DEBUG'
    ,'-D_GLIBCXX_DEBUG_PEDANTIC'
    ,'-D_FORTIFY_SOURCE=2'
    ,'-fno-sanitize-recover'
    ,'-fstack-protector'
    ,'-Wsign-conversion'
    ,'-Weffc++'
  ], language : ['cpp'])
endif

src_unit_test_files = [
    'decoder.cpp'
]
src_dir = 'src'

foreach file : src_unit_test_files
  path = src_dir / file
  exe = executable(file, sources : path,
      include_directories : incdir,
      cpp_args : ['-DUNIT_TEST'],
      dependencies : catch_dep)
  test(file, exe)
endforeach

src_all_files = src_unit_test_files + [
    'encoder.cpp',
    'main.cpp',
]

path_src = []

foreach file : src_all_files
  path_src += src_dir / file
endforeach

executable(program_name, sources : path_src, include_directories : incdir)
