// Copyright (c) 2010 - 2015 Leap Motion. All rights reserved. Proprietary and confidential.
#pragma once

#include <autowiring/ContextMember.h>
#include <autowiring/CoreRunnable.h>

#include <deque>
#include <mutex>

class HttpTransferBase;
class HttpTransferWorker;

class NetworkTransferManager :
  public ContextMember,
  public CoreRunnable
{
  public:
    NetworkTransferManager();
    ~NetworkTransferManager();

    std::shared_ptr<HttpTransferBase> submit(std::shared_ptr<HttpTransferBase> transfer);

  protected:
    bool OnStart(void) override;
    void OnStop(bool graceful) override;

  private:
    enum { MAX_TRANSFER_WORKERS = 3 };

    std::mutex m_mutex;
    std::condition_variable m_cond;
    std::deque<std::shared_ptr<HttpTransferBase>> m_transfers;
    int m_numWaiters = 0;

    // Called by instances of HttpTransferWorker:
    std::shared_ptr<HttpTransferBase> waitForTransfer(const std::chrono::milliseconds& maxWait);

    friend class HttpTransferWorker;
};
