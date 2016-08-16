// Copyright (C) 2012-2016 Leap Motion, Inc. All rights reserved.
#pragma once

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "NetworkStream.h"
#include "CircularBufferStream.h"
#include "Cookie.h"
#include <autowiring/Autowired.h>

#include <atomic>
#include <queue>
#include <curl/curl.h>

class NetworkSessionManager;

class NetworkSession final {
  public:
    NetworkSession();
    ~NetworkSession();
    // Do not allow this object to be copied
    NetworkSession(const NetworkSession& other) = delete;
    NetworkSession& operator=(const NetworkSession& other) = delete;

    std::ostream& sendRequest(HttpRequest& request);
    std::istream& receiveResponse(HttpResponse& response, const std::chrono::milliseconds& timeout = std::chrono::minutes(1));

    bool hasProxy() const;
    bool useNextProxy();

    void cancel();
    bool isCanceled() const;

  private:
    enum State { STATE_INITIALIZING, STATE_RUNNING, STATE_CANCELED, STATE_TERMINATED };

    void setProxyOption();

    void startup();
    void close(bool bad = false);
    void shutdown();

    bool setState(State state);
    bool isState(State state) const;

    size_t readHeader(char* ptr, size_t size, size_t nmemb);
    size_t readContent(char* ptr, size_t size, size_t nmemb);
    size_t writeContent(char* ptr, size_t size, size_t nmemb);
    CURLcode caCertificates(CURL* curl, void* sslCtx);

    static size_t readHeaderStatic(char* ptr, size_t size, size_t nmemb, void* userdata);
    static size_t readContentStatic(char* ptr, size_t size, size_t nmemb, void* userdata);
    static size_t writeContentStatic(char* ptr, size_t size, size_t nmemb, void* userdata);
    static CURLcode caCertificatesStatic(CURL* curl, void* sslCtx, void* parm);

    Autowired<NetworkSessionManager> m_networkSessionManager;
    mutable std::mutex m_proxyMutex;
    std::mutex m_headerMutex;
    std::condition_variable m_headerCondition;
    CURL* m_easy;
    std::queue<Url> m_proxies;
    CircularBufferStream<char> m_outputBuffer;
    CircularBufferStream<char> m_inputBuffer;
    OutputNetworkStream m_outputNetworkStream;
    InputNetworkStream m_inputNetworkStream;
    HttpResponse m_response;
    struct curl_slist *m_list;
    int m_responseStatus;
    bool m_receivedContinue;
    bool m_receivedHeader;
    std::atomic<State> m_state;
    Url m_url;

    friend class NetworkSessionManager;
    friend class HttpTransferWorker;
};
