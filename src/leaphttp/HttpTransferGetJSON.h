// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#pragma once
#include "HttpTransferGetStream.h"
#include "DataStructures/Value.h"

class HttpTransferGetJSON : public HttpTransferGetStream {
public:
  HttpTransferGetJSON(
    const Url& url,
    const std::map<std::string, std::string>& headers,
    const std::function<void(const Value &notesJSON, int error)>& callback
  ) : HttpTransferGetStream(url, m_oss, headers), m_callback(callback) {}

  size_t onRead(const char* buffer, size_t bufferSize) override {
    const auto size = HttpTransferGetStream::onRead(buffer, bufferSize);
    // We are expecting a relatively small buffer, fail if it gets too big
    if (m_oss.tellp() >= std::numeric_limits<uint16_t>::max()) {
      return 0;
    }
    return size;
  }

  void onFinished() override {
    HttpTransferGetStream::onFinished();
    m_callback(Value::FromJSON(m_oss.str()), 0);
  }
  void onCanceled() override {
    HttpTransferGetStream::onCanceled();
    m_callback(Value::FromJSON(m_oss.str()), -1);
  }
  void onError(int error) override {
    HttpTransferGetStream::onError(error);
    if(m_oss && !m_oss.str().empty())
      m_callback(Value::FromJSON(m_oss.str()), error);
    else
      m_callback(Value{}, error);
  }

private:
  std::function<void(const Value& value, int error)> m_callback;
  std::ostringstream m_oss;
};
