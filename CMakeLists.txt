cmake_minimum_required(VERSION 3.12)
project(StrTpl
  VERSION 0.1.0
  LANGUAGES CXX
)
include(GNUInstallDirs)

# Creates a library StrTpl which is an interface (header files only)
add_library(StrTpl INTERFACE)
# If you want to use StrTpl prefer to link against StrTpl using this alias target
add_library(StrTpl::StrTpl ALIAS StrTpl)

# Specify build type ifnot
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
  message(STATUS "CMAKE_BUILD_TYPE not specified: Use Release by default.")
endif(NOT CMAKE_BUILD_TYPE)

# Determine whether this is a standalone project or included by other projects
set(STRTPL_STANDALONE_PROJECT OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(STRTPL_STANDALONE_PROJECT ON)
endif()

# Project options
option(STRTPL_INSTALL "Generate and install StrTpl target" ${STRTPL_STANDALONE_PROJECT})
option(STRTPL_TEST "Build and perform StrTpl tests" ${STRTPL_STANDALONE_PROJECT})

# Setup include directory
add_subdirectory(include)

if(STRTPL_INSTALL)
  install(
    TARGETS StrTpl
    EXPORT StrTplConfig
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    # RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )
  install(
    DIRECTORY include/strtpl
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )
  # Make library importable by other projects
  install(
    EXPORT StrTplConfig
    NAMESPACE StrTpl::
    FILE StrTplConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/StrTpl
    # EXPORT_LINK_INTERFACE_LIBRARIES
  )
endif()

if(STRTPL_TEST)
  include(CTest)
  add_subdirectory(tests)
endif()
