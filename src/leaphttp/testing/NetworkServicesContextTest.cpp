// Copyright (C) 2012-2016 Leap Motion, Inc. All rights reserved.
#include "stdafx.h"
#include <autowiring/C++11/cpp11.h>
#include <leaphttp/NetworkSession.h>
#include <leaphttp/NetworkSessionManager.h>
#include <leaphttp/NetworkTransferManager.h>
#include <leaphttp/HttpTransferDownload.h>
#include <leaphttp/HttpTransferPostString.h>
#include FILESYSTEM_HEADER

#include <gtest/gtest.h>
#include <mutex>
#include <thread>

using std::size_t;

class DefaultHttpRequest :
  public HttpRequest
{
public:
  DefaultHttpRequest(Url url) :
    HttpRequest("Leap", std::move(url))
  {}
};

class NetworkServicesContextTest :
  public testing::Test
{
  public:
    NetworkServicesContextTest() {
      // Force initialization of the path parsing subsystems
      std::filesystem::path p = GetTemporaryPath();
      AddRootCertificateAuthority();
      AutoCurrentContext()->Initiate();
    }

    ~NetworkServicesContextTest() {
      AutoCurrentContext()->SignalShutdown();
      AutoCurrentContext()->Wait();
    }

    static std::wstring MakeRandomName(void) {
      static uint32_t s_counter = 0;
      std::wostringstream oss;
      oss << std::chrono::profiling_clock::now().time_since_epoch().count()
          << "." << ++s_counter;
      return oss.str();
    }

    std::filesystem::path GetTemporaryPath() {
      return { MakeRandomName() };
    }

    std::string GetTemporaryPathStr() {
      auto rv = MakeRandomName();
      return { rv.begin(), rv.end() };
    }

    void AddRootCertificateAuthority() {
      static const std::string cert_DigiCert_Global_Root_CA =
        "-----BEGIN CERTIFICATE-----\n"
        "MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBhMQswCQYDVQQG\n"
        "EwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3d3cuZGlnaWNlcnQuY29tMSAw\n"
        "HgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBDQTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAw\n"
        "MDAwMDBaMGExCzAJBgNVBAYTAlVTMRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3\n"
        "dy5kaWdpY2VydC5jb20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkq\n"
        "hkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsBCSDMAZOn\n"
        "TjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97nh6Vfe63SKMI2tavegw5\n"
        "BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt43C/dxC//AH2hdmoRBBYMql1GNXRor5H\n"
        "4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7PT19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y\n"
        "7vrTC0LUq7dBMtoM1O/4gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQAB\n"
        "o2MwYTAOBgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbRTLtm\n"
        "8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUwDQYJKoZIhvcNAQEF\n"
        "BQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/EsrhMAtudXH/vTBH1jLuG2cenTnmCmr\n"
        "EbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIt\n"
        "tep3Sp+dWOIrWcBAI+0tKIJFPnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886\n"
        "UAb3LujEV0lsYSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n"
        "CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n"
        "-----END CERTIFICATE-----\n";
      m_networkSessionManager->addCaCertificate(cert_DigiCert_Global_Root_CA);
    }

  protected:
    AutoRequired<NetworkSessionManager> m_networkSessionManager;
    AutoRequired<NetworkTransferManager> m_networkTransferManager;
};

class HttpTransferGetTest :
  public HttpTransferBase
{
public:
  HttpTransferGetTest(const Url& url) :
    HttpTransferBase{ { "Leap Motion", url } }
  {
    HttpRequest& request = this->request();
    request.setUrl(url);
    request.setUserAgent("Mozilla/5.0 (Linux; U; Android 2.3; en-us) AppleWebKit/999+ (KHTML, like Gecko) Safari/999.9");
    request.setMethod(HttpRequest::HTTP_GET);
  }
  virtual ~HttpTransferGetTest() {}

  size_t onRead(const char* buffer, size_t bufferSize) override {
    if (m_done) {
      return 0;
    } else {
      m_contentLength += bufferSize;
      return bufferSize;
    }
  }
  void onFinished() override { setDone(true); }
  void onCanceled() override { setDone(false); }
  void onError(int error) override { setDone(false); }

  bool waitForCompletion(const std::chrono::milliseconds& timeout = std::chrono::minutes(1)) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_condition.wait_for(lock, timeout, [this] { return m_done; });
    return m_success && m_done;
  }

  size_t receivedContentLength() const {
    return m_contentLength;
  }

private:
  void setDone(bool success) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_success = success;
    m_done = true;
    m_condition.notify_all();
  }

  mutable std::mutex m_mutex;
  std::condition_variable m_condition;
  size_t m_contentLength = 0;
  bool m_done = false;
  bool m_success = false;
};

TEST_F(NetworkServicesContextTest, VerifyHttpGetTransfers)
{
  const char* sites[] = {
    "http://www.abebooks.com/",
    "http://www.cnn.com/",
    "http://www.msn.com/"
  };
  const int numSites = sizeof(sites)/sizeof(sites[0]);
  std::shared_ptr<HttpTransferGetTest> transfers[numSites];

  for (int i = 0; i < numSites; i++) {
    transfers[i] = std::dynamic_pointer_cast<HttpTransferGetTest>(
        m_networkTransferManager->submit(std::make_shared<HttpTransferGetTest>(Url(sites[i]))));
    EXPECT_NE(transfers[i], nullptr);
  }
  for (int i = 0; i < numSites; i++) {
    if (transfers[i]) {
      EXPECT_TRUE(transfers[i]->waitForCompletion(std::chrono::seconds(30))) << "Failed to complete transfer from " << sites[i];
      EXPECT_EQ(200, transfers[i]->response().status()) << "Unexpected response from " << sites[i];
      EXPECT_GT(transfers[i]->receivedContentLength(), 0U) << "Unexpected content length from " << sites[i];
    }
  }
}

class HttpTransferDownloadTest :
  public HttpTransferDownload
{
public:
  HttpTransferDownloadTest(const Url& url, const std::string& filename) :
    HttpTransferDownload("Leap Motion", url, filename),
    m_filename(filename)
  {}

  virtual ~HttpTransferDownloadTest() {
    try {
      std::filesystem::path file{ std::wstring { m_filename.begin(), m_filename.end() } };
      std::filesystem::remove(file);
    } catch (...) {}
  }

  virtual void onFinished() override { HttpTransferDownload::onFinished(); setDone(true); }
  virtual void onCanceled() override { HttpTransferDownload::onCanceled(); setDone(false); }
  virtual void onError(int error) override { HttpTransferDownload::onError(error); setDone(false); }

  bool waitForCompletion(const std::chrono::milliseconds& timeout = std::chrono::minutes(1)) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_condition.wait_for(
      lock,
      timeout,
      [this] {
        return m_done;
      }
    );
    return m_success;
  }

private:
  void setDone(bool success) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_done = true;
    m_success = success;
    m_condition.notify_all();
  }

  std::string m_filename;
  mutable std::mutex m_mutex;
  std::condition_variable m_condition;
  bool m_done = false;
  bool m_success = false;
};

TEST_F(NetworkServicesContextTest, VerifyHttpDownloadBadFilename)
{
  auto transfer = std::dynamic_pointer_cast<HttpTransferDownloadTest>(
    m_networkTransferManager->submit(
      std::make_shared<HttpTransferDownloadTest>(
        Url("http://www.google.com/"),
        "./this should not exist/google.html"
      )
    )
  );

  ASSERT_FALSE(transfer->waitForCompletion(std::chrono::seconds(30)));
}

TEST_F(NetworkServicesContextTest, VerifyHttpDownloadBadHost)
{
  std::string path = GetTemporaryPathStr();
  auto transfer = std::dynamic_pointer_cast<HttpTransferDownloadTest>(
    m_networkTransferManager->submit(
      std::make_shared<HttpTransferDownloadTest>(
        Url("http://www.invalid-host.unknown/"),
        path
      )
    )
  );

  EXPECT_FALSE(transfer->waitForCompletion(std::chrono::seconds(30)));
}

TEST_F(NetworkServicesContextTest, VerifyHttpDownloadBadUrl)
{
  std::string path = GetTemporaryPathStr();

  auto transfer = std::dynamic_pointer_cast<HttpTransferDownloadTest>(
    m_networkTransferManager->submit(
      std::make_shared<HttpTransferDownloadTest>(
        Url("http://www.google.com/unknown.html"),
        path
      )
    )
  );

  EXPECT_FALSE(transfer->waitForCompletion(std::chrono::seconds(30)));
}

TEST_F(NetworkServicesContextTest, VerifyHttpDownload)
{
  std::string path = GetTemporaryPathStr();

  auto transfer = std::dynamic_pointer_cast<HttpTransferDownloadTest>(
    m_networkTransferManager->submit(
      std::make_shared<HttpTransferDownloadTest>(
        Url("http://www.gstatic.com/webp/gallery/3.jpg"),
        path
      )
    )
  );

  ASSERT_TRUE(transfer->waitForCompletion(std::chrono::seconds(30)));
}

TEST_F(NetworkServicesContextTest, VerifyHttpsGetFailure)
{
  NetworkSession session;
  DefaultHttpRequest request(Url("https://www.google.com/"));

  request.setMethod(HttpRequest::HTTP_GET);
  request.setHeader("Accept", "application/json");
  HttpResponse response;

  std::ostream& ostr = session.sendRequest(request);
  EXPECT_FALSE(ostr.good()); // Cannot write to a GET request

  std::istream& istr = session.receiveResponse(response, std::chrono::minutes(1));
  EXPECT_FALSE(istr.good()); // No CA certificates, will cause a failure
}

TEST_F(NetworkServicesContextTest, VerifyHttpSimultaneousTransfers)
{
  const char* sites[] = {
    "http://www.abebooks.com/",
    "http://www.cnn.com/",
    "http://www.msn.com/"
  };

  for (auto site : sites) {
    NetworkSession session;
    HttpRequest request("Leap Motion", Url{ site });

    request.setMethod(HttpRequest::HTTP_GET);
    request.setUserAgent("Mozilla/5.0 (iPad; U; CPU OS 3_2_1 like Mac OS X; en-us) AppleWebKit/531.21.10 (KHTML, like Gecko) Mobile/7B405");
    HttpResponse response;

    std::ostream& ostr = session.sendRequest(request);
    ASSERT_FALSE(ostr.good()); // Cannot write to a GET request

    std::istream& istr = session.receiveResponse(response, std::chrono::minutes(1));
    ASSERT_TRUE(istr.good()) << "Unexpected stream state " << site;
    ASSERT_EQ(200, response.status()) << "Unexpected response from " << site;
    ASSERT_EQ("OK", response.reason()) << "Unexpected response from " << site;

    char buffer[4096];
    while (istr.good()) {
      istr.read(buffer, sizeof(buffer));
    }
  }
}

TEST_F(NetworkServicesContextTest, VerifyHttpsGet)
{
  NetworkSession session;
  DefaultHttpRequest request(Url("https://warehouse.leapmotion.com/api/v1/apps/consumer-bundle/metadata"));

  request.setMethod(HttpRequest::HTTP_GET);
  request.setHeader("Accept", "application/json");
  HttpResponse response;

  std::ostream& ostr = session.sendRequest(request);
  EXPECT_FALSE(ostr.good()); // Cannot write to a GET request

  std::istream& istr = session.receiveResponse(response, std::chrono::minutes(1));
  EXPECT_TRUE(istr.good());
  EXPECT_EQ(200, response.status());
  EXPECT_EQ("OK", response.reason());

  EXPECT_TRUE(response.hasHeader("Content-Type"));
  // While we are at it, verify that we handle case-insensitive header keys
  EXPECT_TRUE(response.hasHeader("cONTENT-tYPE"));
  EXPECT_EQ("application/json; charset=UTF-8", response.header("Content-Type"));

  char buffer[32], c;

  istr.read(buffer, sizeof(buffer));
  EXPECT_GT(istr.gcount(), 0);
  EXPECT_EQ('{', buffer[0]);

  while (istr.good()) {
    istr.read(buffer, sizeof(buffer));
    if (istr.gcount() <= 0) {
      EXPECT_FALSE(istr.good());
    } else {
      c = buffer[istr.gcount() - 1]; // Hold on to the last byte
    }
  }
  EXPECT_EQ('}', c);
}

TEST_F(NetworkServicesContextTest, VerifyHttpsGetWithCachedCertificates)
{
  NetworkSession session;
  DefaultHttpRequest request(Url("https://warehouse.leapmotion.com/api/v1/apps/consumer-bundle/metadata"));

  request.setMethod(HttpRequest::HTTP_GET);
  request.setHeader("Accept", "application/json");
  HttpResponse response;

  std::ostream& ostr = session.sendRequest(request);
  EXPECT_FALSE(ostr.good()); // Cannot write to a GET request

  std::istream& istr = session.receiveResponse(response, std::chrono::minutes(1));
  EXPECT_TRUE(istr.good());
  EXPECT_EQ(200, response.status());
  EXPECT_EQ("OK", response.reason());
}

TEST_F(NetworkServicesContextTest, VerifyHttpsGetMultipleWithSameSession)
{
  NetworkSession session;
  DefaultHttpRequest request(Url("https://warehouse.leapmotion.com/api/v1/apps/consumer-bundle/metadata"));

  request.setMethod(HttpRequest::HTTP_GET);
  request.setHeader("Accept", "application/json");
  HttpResponse response;

  for (int pass = 0; pass < 4; pass++) { // Run multiple passes with the same session
    std::ostream& ostr = session.sendRequest(request);
    EXPECT_FALSE(ostr.good()); // Cannot write to a GET request

    std::istream& istr = session.receiveResponse(response, std::chrono::minutes(1));
    EXPECT_TRUE(istr.good());
    EXPECT_EQ(200, response.status());
    EXPECT_EQ("OK", response.reason());

    EXPECT_TRUE(response.hasHeader("Content-Type"));
    EXPECT_EQ("application/json; charset=UTF-8", response.header("Content-Type"));

    char buffer[32], c;

    istr.read(buffer, sizeof(buffer));
    EXPECT_GT(istr.gcount(), 0);
    EXPECT_EQ('{', buffer[0]);

    while (istr.good()) {
      istr.read(buffer, sizeof(buffer));
      if (istr.gcount() <= 0) {
        EXPECT_FALSE(istr.good());
      } else {
        c = buffer[istr.gcount() - 1]; // Hold on to the last byte
      }
    }
    EXPECT_EQ('}', c);
  }
}

TEST_F(NetworkServicesContextTest, VerifyHttpRedirect)
{
  NetworkSession session;
  DefaultHttpRequest request(Url("http://warehouse.leapmotion.com/api/v1/apps/consumer-bundle/metadata"));

  request.setMethod(HttpRequest::HTTP_GET);
  request.setHeader("Accept", "application/json");
  HttpResponse response;

  std::ostream& ostr = session.sendRequest(request);
  EXPECT_FALSE(ostr.good()); // Cannot write to a GET request

  std::istream& istr = session.receiveResponse(response, std::chrono::minutes(1));
  EXPECT_TRUE(istr.good());
  EXPECT_EQ(200, response.status());
  EXPECT_EQ("OK", response.reason());

  EXPECT_TRUE(response.hasHeader("Content-Type"));
  EXPECT_EQ("application/json; charset=UTF-8", response.header("Content-Type"));
}

TEST_F(NetworkServicesContextTest, VerifyHttpsMultipleSessions)
{
  NetworkSession session1, session2;
  DefaultHttpRequest request(Url("https://warehouse.leapmotion.com/api/v1/apps/consumer-bundle/metadata"));

  request.setMethod(HttpRequest::HTTP_GET);
  request.setHeader("Accept", "application/json");
  HttpResponse response1, response2;

  std::ostream& ostr1 = session1.sendRequest(request);
  std::ostream& ostr2 = session2.sendRequest(request);
  EXPECT_FALSE(ostr1.good()); // Cannot write to a GET request
  EXPECT_FALSE(ostr2.good()); // Cannot write to a GET request

  std::istream& istr1 = session1.receiveResponse(response1, std::chrono::minutes(1));
  std::istream& istr2 = session2.receiveResponse(response2, std::chrono::minutes(1));
  EXPECT_TRUE(istr1.good());
  EXPECT_TRUE(istr2.good());
  EXPECT_EQ(200, response1.status());
  EXPECT_EQ(200, response2.status());
  EXPECT_EQ("OK", response1.reason());
  EXPECT_EQ("OK", response2.reason());

  EXPECT_TRUE(response1.hasHeader("Content-Type"));
  EXPECT_TRUE(response2.hasHeader("Content-Type"));
  EXPECT_EQ("application/json; charset=UTF-8", response1.header("Content-Type"));
  EXPECT_EQ("application/json; charset=UTF-8", response2.header("Content-Type"));
}

TEST_F(NetworkServicesContextTest, VerifyRepeatedCancel)
{
  for (int i = 0; i < 2; i++) { // Try with a fresh session a couple of times
    NetworkSession session;
    for (int j = 0; j < 2; j++) { // Then use the same session a couple of times
      DefaultHttpRequest request(Url("http://cachefly.cachefly.net/100mb.test"));

      request.setUserAgent("Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.2; Trident/6.0)");
      request.setMethod(HttpRequest::HTTP_GET);
      HttpResponse response;

      std::ostream& ostr = session.sendRequest(request);
      EXPECT_FALSE(ostr.good()); // Cannot write to a GET request

      std::istream& istr = session.receiveResponse(response, std::chrono::minutes(1));
      EXPECT_TRUE(istr.good());
      EXPECT_EQ(200, response.status());
      EXPECT_EQ("OK", response.reason());

      // After a second, cancel the session
      auto f = [&session] {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        session.cancel();
      };
      std::thread thread(f);

      char buffer[4096];
      std::streamsize size = 0;
      while (istr.good()) {
        istr.read(buffer, sizeof(buffer));
        size += istr.gcount();
      }
      // We should have read at least 10,000 bytes by now
      EXPECT_GE(size, 10000) << "Only received " << size << " bytes in half a second";

      // If we only received part of the content, verify that we canceled the
      // session. Otherwise, verify that the transfer wasn't canceled.
      const bool partial = ((size_t)size != response.contentLength());
      EXPECT_EQ(partial, session.isCanceled());

      thread.join();
    }
  }
}
