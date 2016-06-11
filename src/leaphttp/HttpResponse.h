// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#pragma once

#include "HttpHeader.h"
#include "Url.h"

class HttpResponse :
  public HttpHeader
{
public:
  HttpResponse() = default;
  virtual ~HttpResponse() = default;

  Url url() const;
  void setUrl(const Url& url);

  int status() const;
  void setStatus(int status);

  std::string reason() const;
  void setReason(const std::string& reason);

  size_t contentLength() const;
  void setContentLength(size_t length);

private:
  Url m_url;
  std::string m_reason;
  size_t m_contentLength = 0;
  int m_status = 0;
};
