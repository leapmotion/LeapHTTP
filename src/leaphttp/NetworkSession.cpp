// Copyright (C) 2012-2016 Leap Motion, Inc. All rights reserved.
#include "stdafx.h"
#include "NetworkSession.h"
#include "NetworkSessionManager.h"
#include "NetworkTransferManager.h"

#include CHRONO_HEADER
#include <algorithm>
#include <sstream>

NetworkSession::NetworkSession() :
  m_easy(curl_easy_init()),
  m_outputBuffer(4096),
  m_inputBuffer(4096),
  m_outputNetworkStream(m_outputBuffer),
  m_inputNetworkStream(m_inputBuffer),
  m_list(nullptr),
  m_receivedContinue(false),
  m_receivedHeader(false),
  m_state(STATE_TERMINATED)
{
}

NetworkSession::~NetworkSession()
{
  shutdown();
  curl_easy_cleanup(m_easy);
}

std::ostream& NetworkSession::sendRequest(HttpRequest& request)
{
  const Url url = request.url();

  shutdown();
  if (!url.isValid() || !setState(STATE_INITIALIZING))
    return m_outputNetworkStream;

  m_inputBuffer.open();
  m_outputBuffer.open();
  m_outputNetworkStream.clear();
  m_inputNetworkStream.clear();
  m_response = HttpResponse();
  m_receivedContinue = false;
  m_receivedHeader = false;
  m_response.setUrl(url);
  m_proxies = m_networkSessionManager ? m_networkSessionManager->proxies() : std::queue<Url>();

  curl_easy_setopt(m_easy, CURLOPT_URL, url.toString().c_str());
  curl_easy_setopt(m_easy, CURLOPT_VERBOSE, 0L);
  curl_easy_setopt(m_easy, CURLOPT_HEADER, 0L);
  curl_easy_setopt(m_easy, CURLOPT_NOPROGRESS, 1L);
  curl_easy_setopt(m_easy, CURLOPT_NOSIGNAL, 1L);
  curl_easy_setopt(m_easy, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(m_easy, CURLOPT_MAXREDIRS, 15L);
  curl_easy_setopt(m_easy, CURLOPT_HEADERFUNCTION, readHeaderStatic);
  curl_easy_setopt(m_easy, CURLOPT_HEADERDATA, this);
  curl_easy_setopt(m_easy, CURLOPT_READFUNCTION, readContentStatic);
  curl_easy_setopt(m_easy, CURLOPT_READDATA, this);
  curl_easy_setopt(m_easy, CURLOPT_WRITEFUNCTION, writeContentStatic);
  curl_easy_setopt(m_easy, CURLOPT_WRITEDATA, this);
  curl_easy_setopt(m_easy, CURLOPT_SSLCERTTYPE, "PEM");
  curl_easy_setopt(m_easy, CURLOPT_SSL_VERIFYHOST, 2L);
  curl_easy_setopt(m_easy, CURLOPT_SSL_CTX_FUNCTION, caCertificatesStatic);
  curl_easy_setopt(m_easy, CURLOPT_SSL_CTX_DATA, this);
  curl_easy_setopt(m_easy, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
  curl_easy_setopt(m_easy, CURLOPT_USERAGENT, request.userAgent().c_str());
  curl_easy_setopt(m_easy, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
  curl_easy_setopt(m_easy, CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
  curl_easy_setopt(m_easy, CURLOPT_PRIVATE, this);

  setProxyOption();

  // Set the HTTP method
  const std::string method = request.method();
  bool useChunkedTransferEncoding = false;
  bool sendContent = false;
  bool expectContinue = false;

  if (method == HttpRequest::HTTP_GET) {
    curl_easy_setopt(m_easy, CURLOPT_HTTPGET, 1L);
  } else if (method == HttpRequest::HTTP_POST) {
    curl_easy_setopt(m_easy, CURLOPT_POST, 1L);
    curl_easy_setopt(m_easy, CURLOPT_POSTFIELDS, nullptr);
    useChunkedTransferEncoding = request.useChunkedTransferEncoding();
    if (!useChunkedTransferEncoding) {
      long postFieldSize = static_cast<long>(request.contentLength());
      curl_easy_setopt(m_easy, CURLOPT_POSTFIELDSIZE, postFieldSize);
    }
    sendContent = true;
  } else if (!method.empty()) {
    curl_easy_setopt(m_easy, CURLOPT_CUSTOMREQUEST, method.c_str());
    if (method == HttpRequest::HTTP_PUT) {
      curl_easy_setopt(m_easy, CURLOPT_UPLOAD, 1L);
      useChunkedTransferEncoding = request.useChunkedTransferEncoding();
      if (!useChunkedTransferEncoding) {
        long inFileSize = static_cast<long>(request.contentLength());
        curl_easy_setopt(m_easy, CURLOPT_INFILESIZE, inFileSize);
      }
      sendContent = true;
    }
  }
  if (useChunkedTransferEncoding) {
    m_list = curl_slist_append(m_list, "Transfer-Encoding: chunked");
    expectContinue = request.expectContinue();
  }
  if (expectContinue) {
    m_list = curl_slist_append(m_list, "Expect: 100-continue");
  } else {
    // Remove "Expect: 100-continue" entry that is automatically added for POST/PUT calls
    m_list = curl_slist_append(m_list, "Expect:");
  }
  // Append Cookies
  auto cookies = request.cookies();
  if (!cookies.empty()) {
    for (const auto& cookie : cookies) {
      m_list = curl_slist_append(m_list, ("Cookie: " + cookie.toString()).c_str());
    }
  }
  // Append user-specified header fields
  auto entries = request.headers();
  if (!entries.empty()) {
    for (const auto& entry : entries) {
      m_list = curl_slist_append(m_list, entry.c_str());
    }
  }
  curl_easy_setopt(m_easy, CURLOPT_HTTPHEADER, m_list);

  if (m_networkSessionManager && m_networkSessionManager->addNetworkSession(*this)) {
    try {
      if (setState(STATE_RUNNING)) {
        throw std::exception();
      }
      bool closeOutputBuffer = true;
      if (sendContent) {
        if (expectContinue) {
          std::unique_lock<std::mutex> headerLock(m_headerMutex);
          if (!m_receivedContinue) {
            m_headerCondition.wait(headerLock);
          }
          closeOutputBuffer = !m_receivedContinue;
        } else {
          closeOutputBuffer = false;
        }
      }
      if (closeOutputBuffer) {
        m_outputBuffer.close();
        m_outputNetworkStream.setstate(std::ios::eofbit);
      }
    } catch (...) {
      shutdown();
    }
  }

  return m_outputNetworkStream;
}

std::istream& NetworkSession::receiveResponse(HttpResponse& response, const std::chrono::milliseconds& timeout)
{
  if (isState(STATE_RUNNING)) {
    m_outputNetworkStream.flush();
    m_outputBuffer.close(); // No further outgoing data (e.g., POST data)
    {
      std::unique_lock<std::mutex> headerLock(m_headerMutex);
      if (!m_receivedHeader &&
          !m_headerCondition.wait_for(headerLock, timeout, [this] { return m_receivedHeader; })) {
        headerLock.unlock();
        m_inputBuffer.close();
        m_inputNetworkStream.setstate(std::ios::eofbit);
      }
    }
    response = m_response;
  }
  return m_inputNetworkStream;
}

void NetworkSession::cancel()
{
  setState(STATE_CANCELED);
  close();
}

bool NetworkSession::isCanceled() const
{
  return isState(STATE_CANCELED);
}

bool NetworkSession::hasProxy() const
{
  std::lock_guard<std::mutex> proxyLock(m_proxyMutex);

  return !m_proxies.empty();
}

bool NetworkSession::useNextProxy()
{
  std::lock_guard<std::mutex> proxyLock(m_proxyMutex);

  if (m_proxies.empty()) {
    return false;
  }
  m_proxies.pop();
  CURL* easy = curl_easy_duphandle(m_easy);
  if (easy) {
    curl_easy_cleanup(m_easy);
    m_easy = easy;
    setProxyOption();
    return true;
  }
  return false;
}

void NetworkSession::setProxyOption()
{
  std::string proxy;

  while (!m_proxies.empty()) {
    Url url = m_proxies.front();
    if (url.isValid()) {
      proxy = url.toString();
      break;
    }
    m_proxies.pop();
  }
  curl_easy_setopt(m_easy, CURLOPT_PROXY, proxy.c_str());
}

void NetworkSession::startup()
{
  setState(STATE_RUNNING);
}

void NetworkSession::close(bool bad)
{
  if (!isState(STATE_INITIALIZING)) {
    m_outputBuffer.close();
    m_inputBuffer.close();

    std::lock_guard<std::mutex> headerLock(m_headerMutex);
    if (bad) {
      m_inputNetworkStream.setstate(std::ios::badbit | std::ios::eofbit);
    }
    if (!m_receivedHeader) {
      m_inputNetworkStream.setstate(std::ios::eofbit);
      m_receivedHeader = true;
      m_headerCondition.notify_all();
    }
  }
}

void NetworkSession::shutdown()
{
  if (setState(STATE_TERMINATED)) {
    close();

    if (m_networkSessionManager) {
      m_networkSessionManager->removeNetworkSession(*this);
    }
    curl_slist_free_all(m_list);
    m_list = nullptr;

    m_outputNetworkStream.setstate(std::ios::eofbit);
    m_inputNetworkStream.setstate(std::ios::eofbit);
  }
}

bool NetworkSession::setState(State state)
{
  State current = m_state;

  while (state > current || (current == STATE_TERMINATED && state == STATE_INITIALIZING)) {
    if (m_state.compare_exchange_strong(current, state)) {
      return true;
    }
  }
  return false;
}

bool NetworkSession::isState(State state) const
{
  return (m_state == state);
}

size_t NetworkSession::readHeader(char* ptr, size_t size, size_t nmemb)
{
  std::string header = std::string(ptr, size*nmemb);
  std::string::size_type pos;

  if ((pos = header.find(':')) != std::string::npos) {
    std::string key = header.substr(0, pos);
    std::string value = header.substr(pos + 1);
    std::string lower; // A lowercase version of the key

    lower.resize(key.size());
    std::transform(key.begin(), key.end(), lower.begin(), ::tolower);

    value.erase(value.find_last_not_of(" \t\r\n") + 1); // Strip trailing whitespace
    value.erase(0, value.find_first_not_of(" \t")); // Strip leading whitespace

    if (lower == "set-cookie") { // Case-insensitive compare
      if ((pos = value.find('=')) != std::string::npos) {
        // Extract the actual Cookie from the header line
        key = value.substr(0, pos);
        value = value.substr(pos + 1);
        m_response.setCookie(Cookie(key, value));
      }
    } else {
      if (lower == "content-length") { // Case-insensitive compare
        std::istringstream iss(value);
        size_t length = 0;
        iss >> length;
        m_response.setContentLength(length);
      }
      m_response.setHeader(key, value, true);
    }
  } else if (header == "\r\n") {
    if (m_response.status() >= 301 && m_response.status() <= 303 &&
        m_response.hasHeader("Location")) { // We are going to perform a redirect
      Url url;
      const std::string location = m_response.header("Location");

      if (!location.empty() && location[0] == '/') { // Location is a relative path
        url = m_response.url();
        url.setFragment("");
        url.setQuery("");
        url.setPath(location);
        url.setUrl(url.toString()); // Validate of our newly constructed URL
      } else {
        url.setUrl(location);
      }
      // Clear the header and let the redirect happen (keep status and reason)
      static_cast<HttpHeader&>(m_response) = HttpHeader();
      // Update the URL associated with this response to be that of the redirect
      if (url.isValid()) {
        m_response.setUrl(url);
      }
    } else if (m_response.status() != 100) {
      std::lock_guard<std::mutex> headerLock(m_headerMutex);
      m_receivedHeader = true;
      m_headerCondition.notify_all();
    }
  } else if (header.substr(0, 7) == "HTTP/1.") {
    header.erase(header.find_last_not_of(" \t\r\n") + 1); // Strip trailing whitespace
    const char* line = header.c_str() + 7;
    if (line[0] >= '0' && line[0] <= '9' && line[1] == ' ' &&
        line[2] >= '1' && line[2] <= '9' &&
        line[3] >= '0' && line[3] <= '9' &&
        line[4] >= '0' && line[4] <= '9' && line[5] == ' ') { // "HTTP/1.# ### "
      m_response.setStatus((line[2] - '0')*100 + (line[3] - '0')*10 + (line[4] - '0'));
      m_response.setReason(std::string(&line[6]));
      if (m_response.status() == 100) {
        std::lock_guard<std::mutex> headerLock(m_headerMutex);
        m_receivedContinue = true;
        m_headerCondition.notify_all();
      }
    }
  }
  return size*nmemb;
}

size_t NetworkSession::readContent(char* ptr, size_t size, size_t nmemb)
{
  return m_outputBuffer.read(ptr, size*nmemb);
}

size_t NetworkSession::writeContent(char* ptr, size_t size, size_t nmemb)
{
  return m_inputBuffer.write(ptr, size*nmemb);
}

CURLcode NetworkSession::caCertificates(CURL* curl, void* sslCtx)
{
  if (!m_networkSessionManager) {
    return CURLE_SSL_CACERT;
  }
  m_networkSessionManager->applyCaCertificates(reinterpret_cast<SSL_CTX*>(sslCtx));

  return CURLE_OK;
}

//
// Have static methods (with class instance as a parameter) forward the calls on the instance methods
//

size_t NetworkSession::readHeaderStatic(char* ptr, size_t size, size_t nmemb, void* userdata)
{
  return reinterpret_cast<NetworkSession*>(userdata)->readHeader(ptr, size, nmemb);
}

size_t NetworkSession::readContentStatic(char* ptr, size_t size, size_t nmemb, void* userdata)
{
  return reinterpret_cast<NetworkSession*>(userdata)->readContent(ptr, size, nmemb);
}

size_t NetworkSession::writeContentStatic(char* ptr, size_t size, size_t nmemb, void* userdata)
{
  return reinterpret_cast<NetworkSession*>(userdata)->writeContent(ptr, size, nmemb);
}

CURLcode NetworkSession::caCertificatesStatic(CURL* curl, void* sslCtx, void* parm)
{
  return reinterpret_cast<NetworkSession*>(parm)->caCertificates(curl, sslCtx);
}
