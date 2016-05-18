// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#pragma once

#include "HttpTransferGetStream.h"
#include <fstream>

class FileStreamWrapper {
  public:
    FileStreamWrapper(const std::string& filename) : m_ofs(filename.c_str(), std::ofstream::binary) {}
    std::ofstream& ofstream() { return m_ofs; }
  private:
    std::ofstream m_ofs;
};

class HttpTransferDownload : private FileStreamWrapper, public HttpTransferGetStream
{
  public:
    HttpTransferDownload(const Url& url, const std::string& filename) : FileStreamWrapper(filename), HttpTransferGetStream(url, ofstream()) {
      if (!ofstream().good()) {
        cancel();
      }
    }

    virtual void onFinished() override { close(); }
    virtual void onCanceled() override { cancel(); }
    virtual void onError(int error) override { close(); }

    virtual void cancel() override { HttpTransferGetStream::cancel(); close(); }

  private:
    void close() { ofstream().close(); }
};
