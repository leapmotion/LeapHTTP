// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#include "stdafx.h"
#include <leaphttp/Url.h>

#include <gtest/gtest.h>

class UrlTest : public testing::Test
{
  protected:
    bool match(const std::string& url, const std::string& expected = std::string()) {
      std::string result = Url(url).toString();

      if (expected.empty()) {
        return (result == url);
      }
      return (result == expected);
    }

    bool valid(const std::string& url) {
      return Url(url).isValid();
    }
};

TEST_F(UrlTest, VerifyMatching) {
  EXPECT_TRUE(match("http://www.example.com:", "http://www.example.com/"));
  EXPECT_TRUE(match("http://www.example.com:80", "http://www.example.com/"));
  EXPECT_TRUE(match("https://www.example.com:443", "https://www.example.com/"));
  EXPECT_TRUE(match("https://www.example.com:4443", "https://www.example.com:4443/"));
  EXPECT_TRUE(match("ftp://ftp.is.co.za/rfc/rfc1808.txt"));
  EXPECT_TRUE(match("ftp://ftp.example.com/pub/something/"));
  EXPECT_TRUE(match("ftp://tray:5uQQo_f@ftp.example.com:2021", "ftp://tray:5uQQo_f@ftp.example.com:2021/"));
  EXPECT_TRUE(match("http://www.ietf.org/rfc/rfc2396.txt"));
  EXPECT_TRUE(match("ldap://[2001:db8::7]/c=GB?objectClass?one"));
  EXPECT_TRUE(match("ldap://[2001:db8::7]/?objectClass?one"));
  EXPECT_TRUE(match("mailto:John.Doe@example.com"));
  EXPECT_TRUE(match("news:comp.infosystems.www.servers.unix"));
  EXPECT_TRUE(match("tel:+1-816-555-1212"));
  EXPECT_TRUE(match("telnet://192.0.2.16:80/"));
  EXPECT_TRUE(match("urn:oasis:names:specification:docbook:dtd:xml:4.1.2"));
  EXPECT_TRUE(match("http://user:pass@example.com:99/foo;bar?q=a#ref"));
  EXPECT_TRUE(match("file:///C:/foo.txt"));
  EXPECT_TRUE(match("file://server/foo.txt"));
  EXPECT_TRUE(match("http://www.example.com", "http://www.example.com/"));
  EXPECT_TRUE(match("http://user@www.example.com", "http://user@www.example.com/"));
  EXPECT_TRUE(match("http://:pass@www.example.com", "http://:pass@www.example.com/"));
  EXPECT_TRUE(match("http://@www.example.com", "http://www.example.com/"));
  EXPECT_TRUE(match("http://:@www.example.com", "http://www.example.com/"));
  EXPECT_TRUE(match("http://www.example.com/cgi-bin/drawgraph.cgi?type=pie&color=green"));
}

TEST_F(UrlTest, VerifyValidity) {
  EXPECT_TRUE(valid("https://www.example.com/"));
  EXPECT_FALSE(valid("this is not valid"));
  EXPECT_FALSE(valid("https://www@www@example.com"));
  EXPECT_FALSE(valid("https://:www.example.com"));
  EXPECT_FALSE(valid("https://@:www.example.com"));
  EXPECT_FALSE(valid("https://www.example.com:bad_port/"));
}

TEST_F(UrlTest, VerifyEncodingAndDecoding) {
  EXPECT_EQ(Url::encode("Te$ting@!t"), "Te%24ting%40%21t");
  EXPECT_EQ(Url::decode("Te%24ting%40%21t"), "Te$ting@!t");
  Url url("https://www.example.com/");
  url.setUsername(Url::encode("ftp://user:pass@www.username.com"));
  url.setPassword(Url::encode("telnet://:pass@www.password.com"));
  EXPECT_EQ(url.host(), "www.example.com");
  EXPECT_EQ(url.username(), "ftp%3A%2F%2Fuser%3Apass%40www.username.com");
  EXPECT_EQ(url.password(), "telnet%3A%2F%2F%3Apass%40www.password.com");
  EXPECT_EQ(Url::decode(url.username()), "ftp://user:pass@www.username.com");
  EXPECT_EQ(Url::decode(url.password()), "telnet://:pass@www.password.com");
  EXPECT_EQ(Url::encode(url.scheme()), "https");
  EXPECT_EQ(Url::decode(url.scheme()), "https");
  const std::string random("/?:&2\\jI*(%401p~lC*8@'V):f@!-\"=+-_~1[|\x8F\x19\x01\xFF]{};:.<.>?/^?*&%");
  EXPECT_EQ(Url::decode(Url::encode(random)), random);
  EXPECT_EQ(Url::decode("%AJ&Q"), "");
  EXPECT_EQ(Url::decode("1%20%AJ&Q"), "1 ");
  EXPECT_EQ(Url::encode("\xAA\x0D\x0A~\xFE\xED"), "%AA%0D%0A~%FE%ED");
  const std::string valid("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~");
  EXPECT_EQ(Url::encode(valid), valid);
  EXPECT_EQ(Url::encode(Url::decode("%2d%5f%2e%7e")), "-_.~");
}
