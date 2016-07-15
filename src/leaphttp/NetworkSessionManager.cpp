// Copyright (C) 2012-2016 Leap Motion, Inc. All rights reserved.
#include "stdafx.h"
#include "NetworkServices.h"
#include "NetworkSessionManager.h"
#include "NetworkTransferManager.h"

NetworkSessionManager::NetworkSessionManager() :
  BasicThread("NetworkSessionManager"),
  m_multi(nullptr)
{
  // NetworkServices initialize a global state used by libcURL,
  // so it should remain persistent in the global context.
  AutoGlobalContext()->Inject<NetworkServices>();

  m_multi = curl_multi_init();
}

NetworkSessionManager::~NetworkSessionManager()
{
  std::unique_lock<std::recursive_mutex> sessionLock(m_sessionMutex);
  m_sessionCondition.wait(sessionLock, [this] { return m_sessions.empty(); });
  sessionLock.unlock();

  if (curl_multi_cleanup(m_multi) == CURLM_OK) {
    m_multi = nullptr;
  }

  std::lock_guard<std::mutex> certLock(m_mutex);

  for (const auto& caCert : m_caCertificates) {
    X509* cert = caCert.second;
    if (cert) {
      X509_free(cert);
    }
  }
  m_caCertificates.clear();
}

bool NetworkSessionManager::addCaCertificate(const std::string& pem)
{
  std::lock_guard<std::mutex> certLock(m_mutex);

  if (m_caCertificates.find(pem) != m_caCertificates.end()) { // Found?
    return true;
  }
  BIO* bio = BIO_new_mem_buf(const_cast<char*>(pem.c_str()), -1);
  X509* cert = nullptr;
  bool valid = false;

  PEM_read_bio_X509(bio, &cert, 0, nullptr);
  if (cert) {
    m_caCertificates[pem] = cert; // Insert it into the CA certificate cache
    valid = true;
  }
  BIO_free(bio);
  return valid;
}

void NetworkSessionManager::applyCaCertificates(SSL_CTX* sslCtx) const
{
  if (!sslCtx) {
    return;
  }
  std::lock_guard<std::mutex> certLock(m_mutex);

  if (m_caCertificates.empty()) {
    return;
  }
  X509_STORE* store = SSL_CTX_get_cert_store(sslCtx);

  if (!store) {
    return;
  }
  for (const auto& caCert : m_caCertificates) {
    X509* cert = caCert.second;

    if (cert) {
      X509_STORE_add_cert(store, cert);
    }
  }
}

bool NetworkSessionManager::addNetworkSession(NetworkSession& networkSession)
{
  std::lock_guard<std::recursive_mutex> sessionLock(m_sessionMutex);

  if (ShouldStop() ||
      m_sessions.find(&networkSession) != m_sessions.end() || // Already found?
      curl_multi_add_handle(m_multi, networkSession.m_easy) != CURLM_OK) { // Not added?
    return false;
  }
  networkSession.startup();
  m_sessions.insert(&networkSession);
  m_sessionCondition.notify_all();
  return true;
}

bool NetworkSessionManager::removeNetworkSession(NetworkSession& networkSession)
{
  std::lock_guard<std::recursive_mutex> sessionLock(m_sessionMutex);

  if (m_sessions.find(&networkSession) == m_sessions.end() || // Not found?
      curl_multi_remove_handle(m_multi, networkSession.m_easy) != CURLM_OK) { // Not removed?
    return false;
  }
  m_sessions.erase(&networkSession);
  m_sessionCondition.notify_all();
  return true;
}

std::queue<Url> NetworkSessionManager::proxies() const
{
  std::lock_guard<std::mutex> proxyLock(m_mutex);

  return m_proxies;
}

void NetworkSessionManager::setProxies(const std::queue<Url>& proxies)
{
  std::lock_guard<std::mutex> proxyLock(m_mutex);

  m_proxies = proxies;
}

void NetworkSessionManager::Run()
{
  CURLMsg* message;
  CURLMcode status;
  int msgsInQueue, numRunning;

  std::unique_lock<std::recursive_mutex> sessionLock(m_sessionMutex);

  while (!ShouldStop()) {
    long curlTimeout = -1;

    if (m_sessions.empty()) {
      m_sessionCondition.wait(sessionLock, [this] { return (ShouldStop() || !m_sessions.empty()); });
      if (ShouldStop()) {
        break;
      }
    }
    curl_multi_timeout(m_multi, &curlTimeout);
    if (curlTimeout < 0) {
      curlTimeout = 25;
    } else if (curlTimeout > 100) {
      curlTimeout = 100;
    }
    if (curl_multi_wait(m_multi, nullptr, 0, curlTimeout, nullptr) == CURLM_OK) {
      sessionLock.unlock();
      // Allow a session to be removed in another thread
      sessionLock.lock();
      if (ShouldStop()) {
        break;
      }
      while ((status = curl_multi_perform(m_multi, &numRunning)) == CURLM_CALL_MULTI_PERFORM &&
             numRunning > 0) {
        sessionLock.unlock();
        // Allow a session to be removed in another thread
        sessionLock.lock();
        if (ShouldStop()) {
          break;
        }
      }

      while ((message = curl_multi_info_read(m_multi, &msgsInQueue)) != nullptr) {
        if (message->msg == CURLMSG_DONE && message->easy_handle) {
          NetworkSession* networkSession = nullptr;
          curl_easy_getinfo(message->easy_handle, CURLINFO_PRIVATE, &networkSession);
          if (networkSession) {
            long osErrno = 0;
            //
            // If a proxy connection failed, attempt to resubmit the request with the next proxy in the list
            //
            if (ShouldStop() ||
               !networkSession->hasProxy() ||
                networkSession->m_easy != message->easy_handle ||
                curl_easy_getinfo(networkSession->m_easy, CURLINFO_OS_ERRNO, &osErrno) != CURLE_OK || osErrno == 0 ||
                m_sessions.find(networkSession) == m_sessions.end() || // Make sure we are still tracking this session
               !networkSession->useNextProxy() || // Try using the next proxy (if we have one)
                curl_multi_add_handle(m_multi, networkSession->m_easy) != CURLM_OK) { // Now resubmit the request
              networkSession->close(message->data.result != 0); // Non-zero result means bad transfer
            }
          }
        }
      }
    }
  }
}

void NetworkSessionManager::OnStop(bool)
{
  std::lock_guard<std::recursive_mutex> sessionLock(m_sessionMutex);
  for (auto* session : m_sessions) {
    if (session)
      session->cancel();
  }
  m_sessionCondition.notify_all();
}
