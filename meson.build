project(
    'Ettuseus',
    'cpp',
    version: '0.0.1',
    default_options: ['buildtype=release', 'cpp_std=c++23', 'warning_level=3'],
)

lib_include_dir = include_directories('include')

lib_src_files = files('lib/chain.cpp', 'lib/sdr_manager.cpp')

uhd = dependency('uhd')

fmt = dependency('fmt')


main_lib = library(
    'Ettuseus',
    lib_src_files,
    include_directories: lib_include_dir,
    dependencies: [uhd, fmt],
    install: true,
)

pybind11 = dependency('pybind11')

py = import('python').find_installation()

python_deps = [py.dependency(), pybind11]

py.extension_module(
    'ettuseus',
    files('python/bindings.cpp'),
    dependencies: python_deps,
    link_with: [main_lib],
    install: true,
    subdir: 'ettuseus',
    include_directories: lib_include_dir,
)
