project(
    'Ettuseus',
    'cpp',
    version: '0.0.1',
    default_options: ['buildtype=release', 'cpp_std=c++23', 'warning_level=3'],
)

lib_include_dir = include_directories('include')

lib_src_files = files('lib/chain.cpp', 'lib/manager.cpp')

soapy = dependency('SoapySDR')

library(
    'Ettuseus',
    lib_src_files,
    include_directories: lib_include_dir,
    dependencies: [soapy],
)
