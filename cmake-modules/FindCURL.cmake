#.rst
# FindCURL
# ------------
#
# Created by Walter Gray.
# Locate and configure CURL
#
# Interface Targets
# ^^^^^^^^^^^^^^^^^
#   CURL::CURL
#
# Variables
# ^^^^^^^^^
#  CURL_ROOT_DIR
#  CURL_FOUND
#  CURL_INCLUDE_DIR
#  CURL_LIBRARIES
#  CURL_LIBRARY_RELEASE
#  CURL_LIBRARY_DEBUG

include(CMakeFindDependencyMacro)
find_package(OpenSSL REQUIRED)

find_path(
  CURL_ROOT_DIR
  NAMES include/curl/curl.h
  PATH_SUFFIXES curl-${CURL_FIND_VERSION}
  curl
)

set(CURL_INCLUDE_DIR ${CURL_ROOT_DIR}/include)

if(MSVC)
  find_library(CURL_LIBRARY_RELEASE "libcurl.lib" HINTS "${CURL_ROOT_DIR}/lib/release")
  find_library(CURL_LIBRARY_DEBUG "libcurl.lib" HINTS "${CURL_ROOT_DIR}/lib/debug")
else()
  find_library(CURL_LIBRARY_RELEASE "libcurl.a" HINTS "${CURL_ROOT_DIR}/lib")
  find_library(CURL_LIBRARY_DEBUG "libcurl.a" HINTS "${CURL_ROOT_DIR}/lib")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  CURL
  REQUIRED_VARS
    CURL_ROOT_DIR
    CURL_INCLUDE_DIR
    CURL_LIBRARY_RELEASE
    CURL_LIBRARY_DEBUG
)

add_library(CURL::CURL IMPORTED STATIC GLOBAL)
set_property(TARGET CURL::CURL PROPERTY IMPORTED_LOCATION ${CURL_LIBRARY_RELEASE})
set_property(TARGET CURL::CURL PROPERTY IMPORTED_LOCATION_DEBUG ${CURL_LIBRARY_DEBUG})
set_property(TARGET CURL::CURL PROPERTY IMPORTED_LOCATION_RELEASE ${CURL_LIBRARY_RELEASE})
set_property(TARGET CURL::CURL PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CURL_INCLUDE_DIR})
set_property(TARGET CURL::CURL PROPERTY INTERFACE_COMPILE_DEFINITIONS CURL_STATICLIB)
target_link_libraries(CURL::CURL INTERFACE ${OPENSSL_LIBRARIES})
