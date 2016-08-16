// Copyright (C) 2012-2016 Leap Motion, Inc. All rights reserved.
#include "stdafx.h"
#include <leaphttp/CircularBufferStream.h>

#include <thread>
#include <gtest/gtest.h>

class CircularBufferStreamTest : public testing::Test {};

TEST_F(CircularBufferStreamTest, VerifyResults)
{
  CircularBufferStream<char> cbs(8);

  const std::string expected_results = "123456789-123456789-123456789-";
  char buffer[30];
  size_t offset = 0;

  cbs.write("12345", 5);
  offset += cbs.read(buffer + offset, 3);
  cbs.write("6789-", 5);
  offset += cbs.read(buffer + offset, 3);
  offset += cbs.read(buffer + offset, 3);
  cbs.write("12345", 5);
  offset += cbs.read(buffer + offset, 3);
  cbs.write("6789-", 5);
  offset += cbs.read(buffer + offset, 3);
  offset += cbs.read(buffer + offset, 3);
  cbs.write("12345", 5);
  offset += cbs.read(buffer + offset, 3);
  offset += cbs.read(buffer + offset, 3);
  cbs.write("6789-", 5);
  offset += cbs.read(buffer + offset, 3);
  offset += cbs.read(buffer + offset, 3);

  EXPECT_EQ(offset, 30U);
  EXPECT_EQ(expected_results, std::string(buffer, 30));

  cbs.close();

  EXPECT_EQ(cbs.read(buffer, 1), 0U); // Reads should return with a size of zero if the stream is closed
}

TEST_F(CircularBufferStreamTest, VerifyMultithreadedResultsWriteFull)
{
  // Try various circular buffer sizes
  size_t sizes[] = {1, 3, 4, 9, 10, 11, 19, 20, 95, 128, 349, 4096, 65536};

  for (size_t size = 0; size < sizeof(sizes)/sizeof(sizes[0]); size++) {
    CircularBufferStream<char> cbs(sizes[size]);
    uint32_t hash = 0;
    uint32_t length = 0;

    auto consumer = [&cbs, &hash, &length] () {
      char buffer[41];
      size_t n;

      while ((n = cbs.read(buffer, sizeof(buffer))) > 0) {
        for (size_t i = 0; i < n; i++) {
          hash += buffer[i]*37 + length++*101;
        }
        // Wait between read to allow the buffer to hopefully be full for some reads
        std::this_thread::sleep_for(std::chrono::microseconds(50));
      }
    };

    auto producer = [&cbs] () {
      static const char table[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
      char hex[3];
      for (int j = 0; j < 16; j++) {
        for (int i = 0; i < 256; i++) {
          hex[0] = table[i >> 4];
          hex[1] = table[i & 0x0F];
          hex[2] = '.';
          cbs.write(hex, 3);
        }
        hex[0] = '\n';
        cbs.write(hex, 1);
      }
      cbs.close();
    };

    std::thread c(consumer), p(producer);

    c.join();
    p.join();

    EXPECT_EQ(3374121592, hash) << "Hash mismatch with circular buffer size of " << sizes[size];
    EXPECT_EQ(12304U, length) << "Length mismatch with circular buffer size of " << sizes[size];
  }
}

TEST_F(CircularBufferStreamTest, VerifyMultithreadedResultsReadEmpty)
{
  // Try various circular buffer sizes
  size_t sizes[] = {1, 3, 4, 9, 10, 11, 19, 20, 95, 128, 349, 4096, 65536};

  for (size_t size = 0; size < sizeof(sizes)/sizeof(sizes[0]); size++) {
    CircularBufferStream<char> cbs(sizes[size]);
    uint32_t hash = 0;
    uint32_t length = 0;

    auto consumer = [&cbs, &hash, &length] () {
      char buffer[41];
      size_t n;

      while ((n = cbs.read(buffer, sizeof(buffer))) > 0) {
        for (size_t i = 0; i < n; i++) {
          hash += buffer[i]*37 + length++*101;
        }
      }
    };

    auto producer = [&cbs] () {
      static const char table[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
      char hex[3];
      for (int j = 0; j < 16; j++) {
        for (int i = 0; i < 256; i++) {
          hex[0] = table[i >> 4];
          hex[1] = table[i & 0x0F];
          hex[2] = '.';
          cbs.write(hex, 3);
        }
        hex[0] = '\n';
        cbs.write(hex, 1);
        // Wait between writes to allow the buffer to hopefully be empty for some writes
        std::this_thread::sleep_for(std::chrono::microseconds(50));
      }
      cbs.close();
    };

    std::thread c(consumer), p(producer);

    c.join();
    p.join();

    EXPECT_EQ(3374121592, hash) << "Hash mismatch with circular buffer size of " << sizes[size];
    EXPECT_EQ(12304U, length) << "Length mismatch with circular buffer size of " << sizes[size];
  }
}

TEST_F(CircularBufferStreamTest, VerifyNoProducer)
{
  CircularBufferStream<char> cbs(256);
  size_t count;

  auto consumer = [&cbs, &count] () {
    char buffer[40];

    count = cbs.read(buffer, sizeof(buffer));
  };

  std::thread c(consumer);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  cbs.close();

  c.join();

  EXPECT_EQ(count, 0U);
}

TEST_F(CircularBufferStreamTest, VerifyIntStreams)
{
  CircularBufferStream<int> cbs(5);
  int numbers[6] = {12345678, 23456789, 34567890, 45678901, 56789012, 67890123};
  int results[6] = {0, 0, 0, 0, 0, 0};
  size_t written = 0, read = 0;

  written += cbs.write(&numbers[written], 1);
  written += cbs.write(&numbers[written], 3);
  read += cbs.read(&results[read], 3);
  written += cbs.write(&numbers[written], 2);
  read += cbs.read(&results[read], 3);

  EXPECT_EQ(written, 6U);
  EXPECT_EQ(read, 6U);

  for (int i = 0; i < 6; i++) {
    EXPECT_EQ(numbers[i], results[i]);
  }
}
