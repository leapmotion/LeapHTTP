set(
  LeapHTTPTest_SRCS
  CircularBufferStreamTest.cpp
  NetworkServicesContextTest.cpp
  UrlTest.cpp
)

add_pch(LeapHTTPTest_SRCS "stdafx.h" "stdafx.cpp")
add_executable(LeapHTTPTest ${LeapHTTPTest_SRCS} "${CMAKE_SOURCE_DIR}/src/gtest-all-guard.cpp")
target_include_directories(LeapHTTPTest PRIVATE ${OPENSSL_INCLUDE_DIR})
target_link_libraries(
  LeapHTTPTest
  LeapHTTP
  Autowiring::AutoTesting
)

add_test(NAME LeapHTTPTest COMMAND $<TARGET_FILE:LeapHTTPTest>)
