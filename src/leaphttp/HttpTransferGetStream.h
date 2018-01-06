// Copyright (C) 2012-2018 Leap Motion, Inc. All rights reserved.
#pragma once
#include "HttpTransferBase.h"
#include <map>

class HttpTransferGetStream:
  public HttpTransferBase
{
public:
  HttpTransferGetStream(
    std::string userAgent,
    const Url& url,
    std::ostream& os,
    const std::map<std::string, std::string>& headers = {}
  ) :
    HttpTransferBase({ std::move(userAgent), url }),
    m_os(os)
  {
    HttpRequest& request = this->request();
    request.setMethod(HttpRequest::HTTP_GET);
    for (const auto& entry : headers) {
      request.setHeader(entry.first, entry.second);
    }
  }

  virtual ~HttpTransferGetStream() {}

  virtual size_t onRead(const char* buffer, size_t bufferSize) override {
    try {
      if (!m_os.good())
        return 0;

      std::streampos position = m_os.tellp();
      m_os.write(buffer, bufferSize);
      return static_cast<size_t>(m_os.tellp() - position);
    } catch (...) {
      return 0; // Error
    }
  }

private:
  std::ostream& m_os;
};
