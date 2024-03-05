project('cybercobra_compiler', 'cpp', default_options : ['cpp_std=c++23'])

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

compiler_src_unit_test_files = [
    'decoder.cpp',
    'encoder.cpp',
]
src_dir = 'src'

foreach file : compiler_src_unit_test_files
  path = src_dir / file
  exe = executable(file, sources : path,
      include_directories : incdir,
      cpp_args : ['-DUNIT_TEST'],
      dependencies : catch_dep)
  test(file, exe)
endforeach

compiler_src_all_files = compiler_src_unit_test_files + [
    'compiler.cpp',
]

compiler_path_src = []

foreach file : compiler_src_all_files
  compiler_path_src += src_dir / file
endforeach

compiler_name = 'cybercobra_compiler'
executable(compiler_name, sources : compiler_path_src, include_directories : incdir)

preprocessor_src_unit_test_files = [
    'test_preprocessor.cpp',
]

test_dir = 'test'

foreach file : preprocessor_src_unit_test_files
  path = test_dir / file
  exe = executable(file, sources : path,
      include_directories : incdir,
      dependencies : catch_dep)
  test(file, exe)
endforeach

preprocessor_path_src = []
preprocessor_src_all_files = [
    'preprocessor.cpp',
]
foreach file : preprocessor_src_all_files
  preprocessor_path_src += src_dir / file
endforeach

preprocessor_name = 'cybercobra_preprocessor'
executable(preprocessor_name, sources : preprocessor_path_src, include_directories : incdir)
