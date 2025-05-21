project('testl', 'cpp',
  meson_version: '>=0.58.0',
  default_options: [
    'cpp_std=c++20',
    'build.cpp_std=c++20',
    'warning_level=2',
    'strip=true',
    'buildtype=debugoptimized',
    'force_fallback_for=wlroots,libliftoff,vkroots'
  ],
  version : '1.0.0',
  license : 'MIT')

libversion = '1.0.0'
lib_soversion = 1

cppc = meson.get_compiler('cpp')

#add_project_link_arguments(
#  '-static-libgcc',
#  '-static-libstdc++',
#  '-static',
#  language: 'cpp',
#)

add_project_arguments(cppc.get_supported_arguments([
  '-Wno-unused-parameter',
  '-Wno-missing-field-initializers',
  '-Wno-c99-designator',
  '-Wno-invalid-offsetof',
  '-Wno-unused-const-variable',
  '-Wno-volatile', # glm warning
  '-Wno-deprecated-volatile',
  '-Wno-ignored-qualifiers', # reshade warning
  '-Wno-missing-braces',
]), language: 'cpp')

dir_base = meson.current_source_dir()
prj_name = meson.project_name()

out_product_path = 'build/meson-out'

dir_install = join_paths(dir_base, out_product_path, prj_name)
dir_install_headers = join_paths(dir_install)

library_root = 'sources'
source_dir = 'sources/src'
incl_dir = 'sources/include'
# add sources var
sources = []
# add to sources variable
subdir(source_dir)
# add include dir
lib_inc = include_directories(incl_dir)
# create shared library
lib = shared_library(
  meson.project_name(),
  sources,
  version : libversion,
  soversion : lib_soversion,
  name_prefix: 'lib',
  install : true,
  include_directories : lib_inc,
  install_dir: dir_install
  )

install_subdir(incl_dir,
    install_dir: dir_install_headers,
    exclude_files: '*.cpp'
    )
