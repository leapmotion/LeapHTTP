// Copyright (C) 2012-2018 Leap Motion, Inc. All rights reserved.
#pragma once

#include "HttpTransferGetStream.h"
#include <fstream>

class FileStreamWrapper
{
public:
  FileStreamWrapper(const std::string& filename) :
    m_ofs(filename.c_str(), std::ofstream::binary)
  {}

  std::ofstream& ofstream() { return m_ofs; }

private:
  std::ofstream m_ofs;
};

class HttpTransferDownload:
  private FileStreamWrapper,
  public HttpTransferGetStream
{
public:
  HttpTransferDownload(std::string userAgent, const Url& url, const std::string& filename) :
    FileStreamWrapper(filename),
    HttpTransferGetStream(std::move(userAgent), url, ofstream())
  {
    if (!ofstream().good()) {
      cancel();
    }
  }

  void onFinished() override { close(); }
  void onCanceled() override { cancel(); }
  void onError(int error) override { close(); }

  void cancel() override {
    HttpTransferGetStream::cancel();
    close();
  }

private:
  void close() { ofstream().close(); }
};
