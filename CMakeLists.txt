# Build environment
cmake_minimum_required(VERSION 3.10)
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED True)

# Project name and version
project(lampbackup VERSION 1.0.0)
configure_file(config.h.in config.h)

# Executable
add_executable(lampbackup lampbackup.cpp)
target_compile_features(lampbackup PRIVATE cxx_std_17)

# Include directories
target_include_directories(lampbackup PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           ${BOOST_INCLUDEDIR}
                           )
if (WIN32)
target_link_libraries(
  lampbackup 
  $ENV{BOOST_ROOT}\\stage\\lib\\libboost_system-vc142-mt-x64-1_75.lib
  $ENV{BOOST_ROOT}\\stage\\lib\\libboost_program_options-vc142-mt-x64-1_75.lib
  stdc++fs
)
endif (WIN32)

if (UNIX)
target_link_libraries(
  lampbackup 
  ${BOOST_LIBRARYDIR}/libboost_system.a
  ${BOOST_LIBRARYDIR}/libboost_program_options.a
  stdc++fs
)
endif (UNIX)
