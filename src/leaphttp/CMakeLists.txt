set(LeapHTTP_SRCS
  CircularBufferStream.h
  Cookie.cpp
  Cookie.h
  HttpHeader.cpp
  HttpHeader.h
  HttpRequest.cpp
  HttpRequest.h
  HttpResponse.cpp
  HttpResponse.h
  HttpTransferBase.h
  HttpTransferDownload.h
  HttpTransferGetStream.h
  HttpTransferPostString.h
  HttpTransferWorker.cpp
  HttpTransferWorker.h
  NetworkServices.cpp
  NetworkServices.h
  NetworkSession.cpp
  NetworkSession.h
  NetworkSessionManager.cpp
  NetworkSessionManager.h
  NetworkStream.h
  NetworkTransferContext.h
  NetworkTransferManager.cpp
  NetworkTransferManager.h
  Url.cpp
  Url.h
)

add_pch(LeapHTTP_SRCS "stdafx.h" "stdafx.cpp")
add_library(LeapHTTP ${LeapHTTP_SRCS})

target_link_libraries(
  LeapHTTP
  PUBLIC
  CURL::CURL
  ZLIB::ZLIB
  Autowiring::Autowiring
)

if(MSVC)
  target_link_libraries(LeapHTTP PUBLIC ws2_32.lib)
else()
  # Linux still requires -ldl because of how the OpenSSL finder module works.  Remove this as soon as we can.
  target_link_libraries(LeapHTTP INTERFACE dl)
endif()

target_include_directories(
  LeapHTTP
  PRIVATE
  ${OPENSSL_INCLUDE_DIR}
  INTERFACE
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>"
  "$<INSTALL_INTERFACE:include>"
)
add_subdirectory(testing)

#
# Install library
#
install(TARGETS LeapHTTP EXPORT LeapHTTPTargets
  DESTINATION lib
  COMPONENT leaphttp
  CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES}
)
install_headers(TARGET LeapHTTP DESTINATION include/leaphttp COMPONENT leaphttp)
