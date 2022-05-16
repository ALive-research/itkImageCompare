#Original from: https://github.com/MultithreadCorner/MCBooster/blob/87c6fef6a6105f16f85023b14865e72aed85a727/cmake/FindTCLAP.cmake

# - Find TCLAP
# Find the TCLAP headers
#
# TCLAP_INCLUDE_DIR - where to find the TCLAP headers
# TCLAP_FOUND       - True if TCLAP is found

if (TCLAP_INCLUDE_DIR)
  # already in cache, be silent
  set (TCLAP_FIND_QUIETLY TRUE)
endif (TCLAP_INCLUDE_DIR)

# find the headers
find_path (TCLAP_DIR include/tclap/CmdLine.h
  PATHS
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_INSTALL_PREFIX}/include
  )

# handle the QUIETLY and REQUIRED arguments and set TCLAP_FOUND to
# TRUE if all listed variables are TRUE
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (TCLAP "TCLAP (http://tclap.sourceforge.net/) could not be found. Set TCLAP_DIR to point to the source code '-DTCLAP_DIR=/path/to/tclap' to the cmake command." TCLAP_DIR)

if (TCLAP_FOUND)
  set (TCLAP_INCLUDE_DIR ${TCLAP_DIR}/include)
endif (TCLAP_FOUND)

mark_as_advanced(TCLAP_DIR)
