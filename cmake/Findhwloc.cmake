# - Find hwloc 
# Find the hwloc (Portable Hardware Locality) library
# This module defines
#  HWLOC_FOUND, If false, hwloc is unavailable on this system.
#  HWLOC_INCLUDE_DIR, where to find hwloc.h
#  HWLOC_LIBRARIES, the libraries needed to use hwloc

find_path(HWLOC_INCLUDE_DIR hwloc.h)
find_library(HWLOC_LIBRARIES NAMES hwloc)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
	hwloc REQUIRED_VARS HWLOC_LIBRARIES HWLOC_INCLUDE_DIR)

