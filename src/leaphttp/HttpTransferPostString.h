// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#pragma once

#include "HttpTransferBase.h"

class HttpTransferPostString : public HttpTransferBase
{
  public:
    HttpTransferPostString(const Url& url, const std::string& content,
                           const std::map<std::string, std::string>& headers = (std::map<std::string, std::string>())) :
      m_content(content),
      m_offset(0)
    {
      HttpRequest& request = this->request();
      request.setUrl(url);
      request.setContentLength(content.size());
      for (const auto& entry : headers) {
        request.setHeader(entry.first, entry.second);
      }
      request.setMethod(HttpRequest::HTTP_POST);
    }
    virtual ~HttpTransferPostString() {}

    virtual size_t onWrite(char* buffer, size_t bufferSize) override {
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
    size_t m_offset;
};
