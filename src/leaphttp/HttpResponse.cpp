// Copyright (C) 2012-2016 Leap Motion, Inc. All rights reserved.
#include "stdafx.h"
#include "HttpResponse.h"

Url HttpResponse::url() const
{
  return m_url;
}

void HttpResponse::setUrl(const Url& url)
{
  m_url = url;
}

int HttpResponse::status() const
{
  return m_status;
}

void HttpResponse::setStatus(int status)
{
  m_status = status;
}

std::string HttpResponse::reason() const
{
  return m_reason;
}

void HttpResponse::setReason(const std::string& reason)
{
  m_reason = reason;
}

size_t HttpResponse::contentLength() const {
  return m_contentLength;
}

void HttpResponse::setContentLength(size_t length) {
  m_contentLength = length;
}
