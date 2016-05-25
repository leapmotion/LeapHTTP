cmake_minimum_required(VERSION 3.4)
include(version.cmake)
project(LeapHTTP VERSION ${LeapHTTP_VERSION})
include("standard/Standard.cmake")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules")
include(AddPCH)
include(ConditionalSources)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# OpenSSL is retarded, so we have to use a manual path here if the default search fails
set(OPENSSL_USE_STATIC_LIBS TRUE)
set(OPENSSL_ROOT_DIR ${CMAKE_PREFIX_PATH}/openssl)
find_package(OpenSSL REQUIRED)

# Standard dependencies:
find_package(autowiring 1.0.1 REQUIRED)
find_package(ZLIB 1.2.8 REQUIRED)
find_package(CURL 7.36.0 REQUIRED)

# Standard calls
set_standard_output_variables()

# We have unit test projects via googletest, they're added in the places where they are defined
enable_testing()

# Recurse through source directories
include_directories(
  contrib
)
add_subdirectory(contrib)
add_subdirectory(src)

# Now we can generate the version and install stuff
generate_version()
combined_installer(
  VENDOR "Leap Motion"
  CONTACT "cmercenary@gmail.com"
)
