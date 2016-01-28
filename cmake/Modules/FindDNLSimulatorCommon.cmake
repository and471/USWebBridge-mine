# - Try to find DNLSimulatorCommon
# Once done, this will define
#
#  DNLSimulatorCommon_FOUND - system has DNLSimulatorCommon
#  DNLSimulatorCommon_INCLUDE_DIRS - the DNLSimulatorCommon include directories
#  DNLSimulatorCommon_LIBRARIES - link these to use DNLSimulatorCommon

include(LibFindMacros)

# Dependencies
libfind_package(DNLSimulatorCommon DNLSimulatorCommon)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(DNLSimulatorCommon_PKGCONF DNLSimulatorCommon)

# Include dir
find_path(DNLSimulatorCommon_INCLUDE_DIR
  NAMES DNLSimulatorCommon.h
  PATHS ${DNLSimulatorCommon_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
find_library(DNLSimulatorCommon_LIBRARY
  NAMES DNLSimulatorCommon
  PATHS ${DNLSimulatorCommon_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(DNLSimulatorCommon_PROCESS_INCLUDES DNLSimulatorCommon_INCLUDE_DIR Magick_INCLUDE_DIRS)
set(DNLSimulatorCommon_PROCESS_LIBS DNLSimulatorCommon_LIBRARY Magick_LIBRARIES)
libfind_process(DNLSimulatorCommon)
