// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#include "stdafx.h"
#include "HttpHeader.h"

HttpHeader::HttpHeader(std::string userAgent) :
  m_userAgent(std::move(userAgent))
{
}

HttpHeader::~HttpHeader()
{
}

bool HttpHeader::hasHeader(const std::string& key) const
{
  return (m_headers.find(key) != m_headers.end());
}

std::string HttpHeader::header(const std::string& key) const
{
  const auto& found = m_headers.find(key);
  if (found == m_headers.end()) {
    return std::string();
  }
  const auto& values = found->second;
  std::string value;

  for (const auto& entry : values) {
    if (!value.empty()) {
      value += ", ";
    }
    value += entry;
  }
  return value;
}

std::set<std::string> HttpHeader::headers() const
{
  std::set<std::string> entries;

  for (const auto& header : m_headers) {
    const auto& values = header.second;

    if (values.empty()) {
      continue;
    }
    std::string value = header.first + ": ";
    int empty = true;

    for (const auto& entry : values) {
      if (empty) {
        empty = false;
      } else {
        value += ", ";
      }
      value += entry;
    }
    entries.insert(value);
  }
  return entries;
}

void HttpHeader::setHeader(const std::string& key, const std::string& value, bool append)
{
  if (key.empty()) {
    return;
  }
  if (append) {
    auto found = m_headers.find(key);

    if (found != m_headers.end()) {
      found->second.push_back(value);
    } else {
      append = false; // Nothing to append to, just insert it
    }
  }
  if (!append) {
    std::vector<std::string> entries;
    entries.reserve(1);
    entries.push_back(value);
    m_headers[key] = entries;
  }
}

Cookie HttpHeader::cookie(const std::string& name) const
{
  auto found = m_cookies.find(name);

  if (found != m_cookies.end()) {
    return *found;
  }
  return Cookie();
}

bool HttpHeader::hasCookie(const std::string& name) const
{
  return (m_cookies.find(name) != m_cookies.end());
}

std::set<Cookie> HttpHeader::cookies() const
{
  return m_cookies;
}

void HttpHeader::setCookie(const Cookie& cookie)
{
  if (cookie.isValid()) {
    m_cookies.insert(cookie);
  }
}

void HttpHeader::setCookies(const std::set<Cookie>& cookies)
{
  for (const auto& cookie : cookies) {
    if (cookie.isValid()) {
      m_cookies.insert(cookie);
    }
  }
}

std::string HttpHeader::userAgent() const
{
  return m_userAgent;
}

void HttpHeader::setUserAgent(std::string userAgent)
{
  m_userAgent = std::move(userAgent);
}
