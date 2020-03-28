project('cppzip', 'cpp',
  version : '1.0',
  default_options : ['warning_level=3', 'cpp_std=c++14']
)

add_project_arguments('-fvisibility=hidden', language : ['cpp'])

boost_path = '/opt/boost/lib'

boost_fs_lib = 'boost_filesystem'
boost_iostream_lib = 'boost_iostreams'
boost_locale_lib = 'boost_locale'

boost_includes = include_directories('/opt/boost/include')

cpp_compiler = meson.get_compiler('cpp')
boost_fs_dep = cpp_compiler.find_library(boost_fs_lib, dirs : boost_path)
boost_iostream_dep = cpp_compiler.find_library(boost_iostream_lib, dirs : boost_path)
boost_locale_dep = cpp_compiler.find_library(boost_locale_lib, dirs : boost_path)

boost_dep = declare_dependency(dependencies : [boost_fs_dep, boost_locale_dep, boost_iostream_dep], include_directories : boost_includes)
zdep = dependency('zlib', version : '>=1.2.8')

cppzip_interface = include_directories('interface')
cppzip_include = include_directories('include')
cppzip_lib = static_library(
	'cppzip',
	[
		'src/cppzip/v1/zip_archive.cpp',
		'src/cppzip/v1/zip_entry.cpp'
	],
	include_directories : [cppzip_interface, cppzip_include],
	dependencies: [zdep, boost_dep]
)
cppzip_test = executable(
	'cppzip_test',
	[
		'src/main.cpp',
	],
	include_directories : [cppzip_interface],
	link_with: [cppzip_lib],
	dependencies: [zdep, boost_dep]
)
