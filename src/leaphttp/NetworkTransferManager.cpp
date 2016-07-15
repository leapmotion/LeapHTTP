// Copyright (C) 2012-2016 Leap Motion, Inc. All rights reserved.
#include "stdafx.h"

#include "NetworkTransferManager.h"
#include "NetworkTransferContext.h"
#include "NetworkSessionManager.h"
#include "HttpTransferWorker.h"

#include <autowiring/ContextEnumerator.h>

NetworkTransferManager::NetworkTransferManager() :
  ContextMember("NetworkTransferManager")
{
  AutoRequired<NetworkSessionManager>();
}

NetworkTransferManager::~NetworkTransferManager()
{
}

bool NetworkTransferManager::OnStart(void)
{
  return true;
}

void NetworkTransferManager::OnStop(bool graceful)
{
  // Cancel all queued, but not yet being worked on, transfers
  std::lock_guard<std::mutex> lock(m_mutex);
  m_transfers.clear();
  m_cond.notify_all();
}

std::shared_ptr<HttpTransferBase> NetworkTransferManager::submit(std::shared_ptr<HttpTransferBase> transfer)
{
  if (ShouldStop()) {
    return std::shared_ptr<HttpTransferBase>();
  }

  bool createNewContext;
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_transfers.push_back(transfer);
    createNewContext = (m_numWaiters == 0);
    m_cond.notify_one();
  }
  if (createNewContext) {
    auto contexts = ContextEnumeratorT<NetworkTransferContext>(GetContext());
    if (std::distance(contexts.begin(), contexts.end()) < MAX_TRANSFER_WORKERS) {
      CurrentContextPusher pshr(GetContext());
      AutoCreateContextT<NetworkTransferContext> ctxt;
      auto worker = ctxt->Inject<HttpTransferWorker>();
      std::weak_ptr<CoreContext> ctxtWeak = worker->GetContext();
      // If the HttpTransferWorker completes, we want to shutdown the entire sub-context
      worker->AddTeardownListener([this, ctxtWeak] {
        auto ctxt = ctxtWeak.lock();
        if (ctxt) {
          ctxt->SignalShutdown();
        }
      });
      ctxt->Initiate();
    }
  }

  return transfer;
}

std::shared_ptr<HttpTransferBase> NetworkTransferManager::waitForTransfer(const std::chrono::milliseconds& maxWait)
{
  std::unique_lock<std::mutex> lock(m_mutex);

  ++m_numWaiters;
  m_cond.wait_for(lock, maxWait, [this] { return ShouldStop() || !m_transfers.empty(); });
  --m_numWaiters;
  if (ShouldStop() || m_transfers.empty()) {
    return std::shared_ptr<HttpTransferBase>();
  }
  auto transfer = m_transfers.front();
  m_transfers.pop_front();

  return transfer;
}
