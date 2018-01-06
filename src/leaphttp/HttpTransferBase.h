// Copyright (C) 2012-2018 Leap Motion, Inc. All rights reserved.
#pragma once

#include "HttpRequest.h"
#include "HttpResponse.h"
#include <atomic>

class HttpTransferWorker;

class HttpTransferBase
{
public:
  HttpTransferBase(HttpRequest httpRequest) :
    m_httpRequest(std::move(httpRequest))
  {}

  virtual ~HttpTransferBase() = default;

  HttpRequest& request() { return m_httpRequest; }
  HttpResponse& response() { return m_httpResponse; }

  // Callback for methods that submit data
  virtual size_t onWrite(char* buffer, size_t bufferSize) {
    return 0;
  }

  // Callback for methods that retrieve data
  virtual size_t onRead(const char* buffer, size_t bufferSize) {
    return bufferSize;
  }

  // Called when the transfer finishes successfully
  virtual void onFinished() {}
  // Called if the transfer is canceled before finishing
  virtual void onCanceled() {}
  // Called when there is an error with the transfer
  virtual void onError(int error) {}

  virtual void cancel() { m_canceled = true; }
  bool isCanceled() const { return m_canceled; }

private:
  HttpRequest m_httpRequest;
  HttpResponse m_httpResponse;

  std::atomic<bool> m_canceled{ false };

  friend class HttpTransferWorker;
};
