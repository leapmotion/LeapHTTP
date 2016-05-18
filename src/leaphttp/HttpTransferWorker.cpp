// Copyright (c) 2010 - 2014 Leap Motion. All rights reserved. Proprietary and confidential.
#include "stdafx.h"

#include "HttpTransferWorker.h"
#include "HttpTransferBase.h"
#include "NetworkSession.h"
#include "NetworkTransferManager.h"

void HttpTransferWorker::Run()
{
  char buffer[4096];

  while (!ShouldStop()) {
    AutowiredFast<NetworkTransferManager> manager;
    if (!manager) {
      break;
    }
    auto transfer = manager->waitForTransfer(std::chrono::seconds(60));
    if (ShouldStop() || !transfer) {
      break;
    }
    try {
      // Create a new session
      auto session = std::make_shared<NetworkSession>();
      if (!session) {
        break;
      }
      if (transfer->isCanceled()) {
        session->cancel();
        throw std::exception();
      }

      // Send the HTTP Request
      std::ostream& ostr = session->sendRequest(transfer->m_httpRequest);
      while (ostr.good()) {
        if (ShouldStop()) {
          throw std::exception();
        }
        if (transfer->isCanceled()) {
          session->cancel();
          throw std::exception();
        }
        size_t size = transfer->onWrite(buffer, sizeof(buffer));
        if (size == 0) {
          break;
        }
        ostr.write(buffer, size);
      }

      // Receive the HTTP Response
      std::istream& istr = session->receiveResponse(transfer->m_httpResponse, std::chrono::seconds(15));
      int status = transfer->m_httpResponse.status();
      if (status < 200 || status >= 300) {
        session.reset();
        transfer->onError(status);
      } else {
        // Process any content
        while (istr.good()) {
          if (ShouldStop()) {
            throw std::exception();
          }
          if (transfer->isCanceled()) {
            session->cancel();
            throw std::exception();
          }
          istr.read(buffer, sizeof(buffer));
          size_t size = static_cast<size_t>(istr.gcount());
          if (size > 0) {
            size_t remaining = size;
            while (remaining > 0) {
              size = transfer->onRead(buffer + size - remaining, remaining);
              if (size == 0) {
                throw std::exception();
              }
              if (size >= remaining) {
                break;
              }
              remaining -= size;
            }
          }
        }
        if (session->isCanceled()) {
          throw std::exception();
        }
        const bool isBad = istr.bad();
        session.reset(); // invalidates istr

        if (isBad) {
          transfer->onError(-1);
        } else {
          transfer->onFinished();
        }
      }
    } catch (...) {
      transfer->onCanceled();
    }
    std::memset(buffer, 0, sizeof(buffer));
  }
}
