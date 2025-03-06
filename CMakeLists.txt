cmake_minimum_required(VERSION 3.21)
project(SKETCHKIT)

# include the fips main cmake file
get_filename_component(FIPS_ROOT_DIR "../fips" ABSOLUTE)
include("${FIPS_ROOT_DIR}/cmake/fips.cmake")

fips_setup()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (FIPS_WINDOWS)
add_compile_options(/Zc:preprocessor)
link_directories(${CMAKE_SOURCE_DIR}/libs/win64)
endif()

fips_begin_lib(sk)
  fips_src(code GROUP code)
  target_include_directories(sk PUBLIC code)
  fips_libs(bgfx bimg bx)
fips_end_lib()

fips_ide_group(apps)
add_subdirectory(apps)
