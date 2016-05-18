// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#pragma once

#include "NetworkSession.h"
#include <autowiring/BasicThread.h>

#include <string>
#include <map>

#include <openssl/ssl.h>
#include <curl/curl.h>

class NetworkSessionManager :
  public BasicThread
{
  public:
    NetworkSessionManager();
    virtual ~NetworkSessionManager();

    bool addCaCertificate(const std::string& pem);
    void applyCaCertificates(SSL_CTX* sslCtx) const;

    bool addNetworkSession(NetworkSession& networkSession);
    bool removeNetworkSession(NetworkSession& networkSession);

    std::queue<Url> proxies() const;
    void setProxies(const std::queue<Url>& proxies);

  private:
    mutable std::mutex m_mutex;
    std::recursive_mutex m_sessionMutex;
    std::condition_variable_any m_sessionCondition;
    CURL* m_multi;

    std::set<NetworkSession*> m_sessions;
    std::map<std::string, X509*> m_caCertificates;
    std::queue<Url> m_proxies;

    void Run() override;
    void OnStop(bool graceful) override;
};
