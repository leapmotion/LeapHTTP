// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#pragma once

#include "HttpHeader.h"
#include "Url.h"

class HttpRequest :
  public HttpHeader
{
public:
  HttpRequest(std::string userAgent, const Url& url, const std::string& method = HTTP_GET);
  virtual ~HttpRequest();

  Url url() const;
  void setUrl(const Url& url);

  std::string method() const;
  void setMethod(const std::string& method);

  bool useChunkedTransferEncoding() const;

  size_t contentLength() const;
  void setContentLength(size_t contentLength);

  bool expectContinue();
  void setExpectContinue(bool expect);

  static const std::string HTTP_DELETE;
  static const std::string HTTP_GET;
  static const std::string HTTP_POST;
  static const std::string HTTP_PUT;

private:
  Url m_url;
  std::string m_method;
  size_t m_contentLength;
  bool m_useChunkedTransferEncoding;
  bool m_expectContinue;
};
