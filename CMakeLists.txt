cmake_minimum_required(VERSION 3.30 FATAL_ERROR)

set(CMAKE_DISABLE_IN_SOURCE_BUILDS ON)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")

project(tpkg)

set(CMAKE_CXX_CLANG_TIDY clang-tidy -p ${CMAKE_BINARY_DIR})
set(CMAKE_CXX_MODULE_STD ON)

add_executable(
	tpkg
	main.cxx
)
target_compile_features(
	tpkg
	PUBLIC
	cxx_std_26
)

install(TARGETS tpkg) 
