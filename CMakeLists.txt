# Build environment
cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Project name and version
project(lampbackup VERSION 1.0.0)
configure_file(config.h.in config.h)

# Executable
add_executable(lampbackup lampbackup.cpp)

# Include directories
target_include_directories(lampbackup PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           $ENV{BOOST_ROOT}
                           )

target_link_libraries(
  lampbackup 
  $ENV{BOOST_ROOT}\\stage\\lib\\libboost_system-vc142-mt-x64-1_75.lib
  $ENV{BOOST_ROOT}\\stage\\lib\\libboost_program_options-vc142-mt-x64-1_75.lib
)