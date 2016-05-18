// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#pragma once

#include "HttpTransferBase.h"

#include <map>

class HttpTransferGetStream : public HttpTransferBase
{
  public:
    HttpTransferGetStream(const Url& url, std::ostream& os,
                          const std::map<std::string, std::string>& headers = (std::map<std::string, std::string>())) :
      m_os(os)
    {
      HttpRequest& request = this->request();
      request.setUrl(url);
      request.setMethod(HttpRequest::HTTP_GET);
      for (const auto& entry : headers) {
        request.setHeader(entry.first, entry.second);
      }
    }
    virtual ~HttpTransferGetStream() {}

    virtual size_t onRead(const char* buffer, size_t bufferSize) override {
      try {
        if (m_os.good()) {
          std::streampos position = m_os.tellp();
          m_os.write(buffer, bufferSize);
          return static_cast<size_t>(m_os.tellp() - position);
        }
      } catch (...) {}
      return 0; // Error
    }

  private:
    std::ostream& m_os;
};
