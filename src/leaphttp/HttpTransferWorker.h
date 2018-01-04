// Copyright (C) 2012-2018 Leap Motion, Inc. All rights reserved.
#pragma once
#include <autowiring/BasicThread.h>

class HttpTransferBase;

class HttpTransferWorker:
  public BasicThread
{
public:
  HttpTransferWorker() :
    BasicThread("HttpTransferWorker")
  {}
  ~HttpTransferWorker() {}

  void ProcessOne(HttpTransferBase& transfer);

  void Run() override;
};
