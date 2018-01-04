// Copyright (C) 2012-2018 Leap Motion, Inc. All rights reserved.
#pragma once
#include "HttpTransferBase.h"
#include <map>

class HttpTransferPostString :
  public HttpTransferBase
{
public:
  HttpTransferPostString(
    std::string userAgent,
    const Url& url,
    const std::string& content,
    const std::map<std::string, std::string>& headers = {}
  ) :
    HttpTransferBase{ {std::move(userAgent), url} },
    m_content(content)
  {
    HttpRequest& request = this->request();
    request.setUrl(url);
    request.setContentLength(content.size());
    for (const auto& entry : headers) {
      request.setHeader(entry.first, entry.second);
    }
    request.setMethod(HttpRequest::HTTP_POST);
  }
  HttpTransferPostString(
    std::string userAgent,
    const Url& url,
    std::string&& content,
    const std::map<std::string, std::string>& headers = {}
  ) :
    HttpTransferBase{ {std::move(userAgent), url} },
    m_content(std::move(content))
  {
    HttpRequest& request = this->request();
    request.setUrl(url);
    request.setContentLength(content.size());
    for (const auto& entry : headers) {
      request.setHeader(entry.first, entry.second);
    }
    request.setMethod(HttpRequest::HTTP_POST);
  }
  virtual ~HttpTransferPostString() = default;

  size_t onWrite(char* buffer, size_t bufferSize) override {
    size_t blockSize = m_content.size() - m_offset;
    if (blockSize > bufferSize) {
      blockSize = bufferSize;
    }
    if (blockSize > 0) {
      std::memcpy(buffer, m_content.c_str() + m_offset, blockSize);
      m_offset += blockSize;
    }
    return blockSize;
  }

private:
  std::string m_content;
  size_t m_offset = 0;
};
