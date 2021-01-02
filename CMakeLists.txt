# Build environment
cmake_minimum_required(VERSION 3.10)

set (APP_NAME "lampbackup")
set (VERSION "1.0.0")

# Project name and version
project(${APP_NAME} VERSION ${VERSION})
configure_file(config.h.in config.h)

# Executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
add_executable(${APP_NAME} main.cpp LAMPbackup.cpp)
install(TARGETS ${APP_NAME} DESTINATION bin)
target_compile_features(${APP_NAME} PRIVATE cxx_std_17)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
      "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif(NOT CMAKE_BUILD_TYPE)
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")


# Include directories
target_include_directories(${APP_NAME} PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           ${BOOST_INCLUDEDIR}
                           )
target_link_libraries(
  ${APP_NAME} 
  ${BOOST_LIBRARYDIR}/libboost_system.a
  ${BOOST_LIBRARYDIR}/libboost_program_options.a
  stdc++fs
)

# RPM package stuff
set(CMAKE_INSTALL_PREFIX "/usr")
set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_GENERATOR "RPM")
set(CPACK_PACKAGE_NAME ${APP_NAME})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "LAMP website backup utility")
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_RPM_PACKAGE_GROUP "Network")
set(CPACK_PACKAGE_CONTACT "Chris Juszak")
set(CPACK_PACKAGE_VENDOR "GeoHub")
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_URL "https://github.com/juszak/LAMPmanage")
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_BUILD_TYPE}")
set(CPACK_RPM_PACKAGE_REQUIRES "mysql, tar, gzip")
set(CPACK_RPM_PACKAGE_AUTOREQPROV "NO")
execute_process(COMMAND bash -c "echo $(lsb_release -sr | cut -d'.' -f 1)" OUTPUT_VARIABLE APP_OS OUTPUT_STRIP_TRAILING_WHITESPACE)
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.el${APP_OS}.${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_BUILD_TYPE}")
include(CPack)
