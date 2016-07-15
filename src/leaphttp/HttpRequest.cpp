// Copyright (C) 2012-2016 Leap Motion, Inc. All rights reserved.
#include "stdafx.h"
#include "HttpRequest.h"
#include "Cookie.h"

HttpRequest::HttpRequest(std::string userAgent, const Url& url, const std::string& method) :
  HttpHeader(std::move(userAgent)),
  m_url(url),
  m_method(method),
  m_contentLength(0),
  m_useChunkedTransferEncoding(true),
  m_expectContinue(true)
{
}

HttpRequest::~HttpRequest()
{
}

Url HttpRequest::url() const
{
  return m_url;
}

void HttpRequest::setUrl(const Url& url)
{
  m_url = url;
}

std::string HttpRequest::method() const
{
  return m_method;
}

void HttpRequest::setMethod(const std::string& method)
{
  m_method = method;
}

bool HttpRequest::useChunkedTransferEncoding() const
{
  return m_useChunkedTransferEncoding;
}

size_t HttpRequest::contentLength() const
{
  return m_contentLength;
}

void HttpRequest::setContentLength(size_t contentLength)
{
  m_contentLength = contentLength;
  m_useChunkedTransferEncoding = false;
}

bool HttpRequest::expectContinue()
{
  return (m_expectContinue && m_useChunkedTransferEncoding);
}

void HttpRequest::setExpectContinue(bool expect)
{
  m_expectContinue = expect;
}

const std::string HttpRequest::HTTP_DELETE = "DELETE";
const std::string HttpRequest::HTTP_GET = "GET";
const std::string HttpRequest::HTTP_POST = "POST";
const std::string HttpRequest::HTTP_PUT = "PUT";
