// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#pragma once

#include <autowiring/BasicThread.h>

class HttpTransferWorker:
  public BasicThread
{
public:
  HttpTransferWorker() :
    BasicThread("HttpTransferWorker")
  {}
  ~HttpTransferWorker() {}

  void Run() override;
};
