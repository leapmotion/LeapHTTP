// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
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
  m_state(STATE_INITIALIZING)
{
  m_networkSessionManager.NotifyWhenAutowired([this] {
    static const std::string cert_DigiCert_High_Assurance_EV_Root_CA =
      "-----BEGIN CERTIFICATE-----\n"
      "MIIDxTCCAq2gAwIBAgIQAqxcJmoLQJuPC3nyrkYldzANBgkqhkiG9w0BAQUFADBsMQswCQYDVQQG\n"
      "EwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3d3cuZGlnaWNlcnQuY29tMSsw\n"
      "KQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5jZSBFViBSb290IENBMB4XDTA2MTExMDAwMDAw\n"
      "MFoXDTMxMTExMDAwMDAwMFowbDELMAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZ\n"
      "MBcGA1UECxMQd3d3LmRpZ2ljZXJ0LmNvbTErMCkGA1UEAxMiRGlnaUNlcnQgSGlnaCBBc3N1cmFu\n"
      "Y2UgRVYgUm9vdCBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbM5XPm+9S75S0t\n"
      "Mqbf5YE/yc0lSbZxKsPVlDRnogocsF9ppkCxxLeyj9CYpKlBWTrT3JTWPNt0OKRKzE0lgvdKpVMS\n"
      "OO7zSW1xkX5jtqumX8OkhPhPYlG++MXs2ziS4wblCJEMxChBVfvLWokVfnHoNb9Ncgk9vjo4UFt3\n"
      "MRuNs8ckRZqnrG0AFFoEt7oT61EKmEFBIk5lYYeBQVCmeVyJ3hlKV9Uu5l0cUyx+mM0aBhakaHPQ\n"
      "NAQTXKFx01p8VdteZOE3hzBWBOURtCmAEvF5OYiiAhF8J2a3iLd48soKqDirCmTCv2ZdlYTBoSUe\n"
      "h10aUAsgEsxBu24LUTi4S8sCAwEAAaNjMGEwDgYDVR0PAQH/BAQDAgGGMA8GA1UdEwEB/wQFMAMB\n"
      "Af8wHQYDVR0OBBYEFLE+w2kD+L9HAdSYJhoIAu9jZCvDMB8GA1UdIwQYMBaAFLE+w2kD+L9HAdSY\n"
      "JhoIAu9jZCvDMA0GCSqGSIb3DQEBBQUAA4IBAQAcGgaX3NecnzyIZgYIVyHbIUf4KmeqvxgydkAQ\n"
      "V8GK83rZEWWONfqe/EW1ntlMMUu4kehDLI6zeM7b41N5cdblIZQB2lWHmiRk9opmzN6cN82oNLFp\n"
      "myPInngiK3BD41VHMWEZ71jFhS9OMPagMRYjyOfiZRYzy78aG6A9+MpeizGLYAiJLQwGXFK3xPkK\n"
      "mNEVX58Svnw2Yzi9RKR/5CYrCsSXaQ3pjOLAEFe4yHYSkVXySGnYvCoCWw9E1CAx2/S6cCZdkGCe\n"
      "vEsXCS+0yx5DaMkHJ8HSXPfqIbloEpw8nL+e/IBcm2PN7EeqJSdnoDfzAIJ9VNep+OkuE6N36B9K\n"
      "-----END CERTIFICATE-----\n";

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

    static const std::string cert_Entrust_net_Secure_Server_CA =
      "-----BEGIN CERTIFICATE-----\n"
      "MIIE2DCCBEGgAwIBAgIEN0rSQzANBgkqhkiG9w0BAQUFADCBwzELMAkGA1UEBhMCVVMxFDASBgNV\n"
      "BAoTC0VudHJ1c3QubmV0MTswOQYDVQQLEzJ3d3cuZW50cnVzdC5uZXQvQ1BTIGluY29ycC4gYnkg\n"
      "cmVmLiAobGltaXRzIGxpYWIuKTElMCMGA1UECxMcKGMpIDE5OTkgRW50cnVzdC5uZXQgTGltaXRl\n"
      "ZDE6MDgGA1UEAxMxRW50cnVzdC5uZXQgU2VjdXJlIFNlcnZlciBDZXJ0aWZpY2F0aW9uIEF1dGhv\n"
      "cml0eTAeFw05OTA1MjUxNjA5NDBaFw0xOTA1MjUxNjM5NDBaMIHDMQswCQYDVQQGEwJVUzEUMBIG\n"
      "A1UEChMLRW50cnVzdC5uZXQxOzA5BgNVBAsTMnd3dy5lbnRydXN0Lm5ldC9DUFMgaW5jb3JwLiBi\n"
      "eSByZWYuIChsaW1pdHMgbGlhYi4pMSUwIwYDVQQLExwoYykgMTk5OSBFbnRydXN0Lm5ldCBMaW1p\n"
      "dGVkMTowOAYDVQQDEzFFbnRydXN0Lm5ldCBTZWN1cmUgU2VydmVyIENlcnRpZmljYXRpb24gQXV0\n"
      "aG9yaXR5MIGdMA0GCSqGSIb3DQEBAQUAA4GLADCBhwKBgQDNKIM0VBuJ8w+vN5Ex/68xYMmo6LIQ\n"
      "aO2f55M28Qpku0f1BBc/I0dNxScZgSYMVHINiC3ZH5oSn7yzcdOAGT9HZnuMNSjSuQrfJNqc1lB5\n"
      "gXpa0zf3wkrYKZImZNHkmGw6AIr1NJtl+O3jEP/9uElY3KDegjlrgbEWGWG5VLbmQwIBA6OCAdcw\n"
      "ggHTMBEGCWCGSAGG+EIBAQQEAwIABzCCARkGA1UdHwSCARAwggEMMIHeoIHboIHYpIHVMIHSMQsw\n"
      "CQYDVQQGEwJVUzEUMBIGA1UEChMLRW50cnVzdC5uZXQxOzA5BgNVBAsTMnd3dy5lbnRydXN0Lm5l\n"
      "dC9DUFMgaW5jb3JwLiBieSByZWYuIChsaW1pdHMgbGlhYi4pMSUwIwYDVQQLExwoYykgMTk5OSBF\n"
      "bnRydXN0Lm5ldCBMaW1pdGVkMTowOAYDVQQDEzFFbnRydXN0Lm5ldCBTZWN1cmUgU2VydmVyIENl\n"
      "cnRpZmljYXRpb24gQXV0aG9yaXR5MQ0wCwYDVQQDEwRDUkwxMCmgJ6AlhiNodHRwOi8vd3d3LmVu\n"
      "dHJ1c3QubmV0L0NSTC9uZXQxLmNybDArBgNVHRAEJDAigA8xOTk5MDUyNTE2MDk0MFqBDzIwMTkw\n"
      "NTI1MTYwOTQwWjALBgNVHQ8EBAMCAQYwHwYDVR0jBBgwFoAU8BdiE1U9s/8KAGv7UISX8+1i0Bow\n"
      "HQYDVR0OBBYEFPAXYhNVPbP/CgBr+1CEl/PtYtAaMAwGA1UdEwQFMAMBAf8wGQYJKoZIhvZ9B0EA\n"
      "BAwwChsEVjQuMAMCBJAwDQYJKoZIhvcNAQEFBQADgYEAkNwwAvpkdMKnCqV8IY00F6j7Rw7/JXyN\n"
      "Ewr75Ji174z4xRAN95K+8cPV1ZVqBLssziY2ZcgxxufuP+NXdYR6Ee9GTxj005i7qIcyunL2POI9\n"
      "n9cd2cNgQ4xYDiKWL2KjLB+6rQXvqzJ4h6BUcxm1XAX5Uj5tLUUL9wqT6u0G+bI=\n"
      "-----END CERTIFICATE-----\n";

    static const std::string cert_Entrust_net_Premium_2048_Secure_Server_CA =
      "-----BEGIN CERTIFICATE-----\n"
      "MIIEXDCCA0SgAwIBAgIEOGO5ZjANBgkqhkiG9w0BAQUFADCBtDEUMBIGA1UEChMLRW50cnVzdC5u\n"
      "ZXQxQDA+BgNVBAsUN3d3dy5lbnRydXN0Lm5ldC9DUFNfMjA0OCBpbmNvcnAuIGJ5IHJlZi4gKGxp\n"
      "bWl0cyBsaWFiLikxJTAjBgNVBAsTHChjKSAxOTk5IEVudHJ1c3QubmV0IExpbWl0ZWQxMzAxBgNV\n"
      "BAMTKkVudHJ1c3QubmV0IENlcnRpZmljYXRpb24gQXV0aG9yaXR5ICgyMDQ4KTAeFw05OTEyMjQx\n"
      "NzUwNTFaFw0xOTEyMjQxODIwNTFaMIG0MRQwEgYDVQQKEwtFbnRydXN0Lm5ldDFAMD4GA1UECxQ3\n"
      "d3d3LmVudHJ1c3QubmV0L0NQU18yMDQ4IGluY29ycC4gYnkgcmVmLiAobGltaXRzIGxpYWIuKTEl\n"
      "MCMGA1UECxMcKGMpIDE5OTkgRW50cnVzdC5uZXQgTGltaXRlZDEzMDEGA1UEAxMqRW50cnVzdC5u\n"
      "ZXQgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkgKDIwNDgpMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8A\n"
      "MIIBCgKCAQEArU1LqRKGsuqjIAcVFmQqK0vRvwtKTY7tgHalZ7d4QMBzQshowNtTK91euHaYNZOL\n"
      "Gp18EzoOH1u3Hs/lJBQesYGpjX24zGtLA/ECDNyrpUAkAH90lKGdCCmziAv1h3edVc3kw37XamSr\n"
      "hRSGlVuXMlBvPci6Zgzj/L24ScF2iUkZ/cCovYmjZy/Gn7xxGWC4LeksyZB2ZnuU4q941mVTXTzW\n"
      "nLLPKQP5L6RQstRIzgUyVYr9smRMDuSYB3Xbf9+5CFVghTAp+XtIpGmG4zU/HoZdenoVve8AjhUi\n"
      "VBcAkCaTvA5JaJG/+EfTnZVCwQ5N328mz8MYIWJmQ3DW1cAH4QIDAQABo3QwcjARBglghkgBhvhC\n"
      "AQEEBAMCAAcwHwYDVR0jBBgwFoAUVeSB0RGAvtiJuQijMfmhJAkWuXAwHQYDVR0OBBYEFFXkgdER\n"
      "gL7YibkIozH5oSQJFrlwMB0GCSqGSIb2fQdBAAQQMA4bCFY1LjA6NC4wAwIEkDANBgkqhkiG9w0B\n"
      "AQUFAAOCAQEAWUesIYSKF8mciVMeuoCFGsY8Tj6xnLZ8xpJdGGQC49MGCBFhfGPjK50xA3B20qMo\n"
      "oPS7mmNz7W3lKtvtFKkrxjYR0CvrB4ul2p5cGZ1WEvVUKcgF7bISKo30Axv/55IQh7A6tcOdBTcS\n"
      "o8f0FbnVpDkWm1M6I5HxqIKiaohowXkCIryqptau37AUX7iH0N18f3v/rxzP5tsHrV7bhZ3QKw0z\n"
      "2wTR5klAEyt2+z7pnIkPFc4YsIV4IU9rTw76NmfNB/L/CNDi3tm/Kq+4h4YhPATKt5Rof8886ZjX\n"
      "OP/swNlQ8C5LWK5Gb9Auw2DaclVyvUxFnmG6v4SBkgPR0ml8xQ==\n"
      "-----END CERTIFICATE-----\n";

    static const std::string cert_Verisign_Class_3_Public_Primary_Certification_Authority =
      "-----BEGIN CERTIFICATE-----\n"
      "MIICPDCCAaUCEDyRMcsf9tAbDpq40ES/Er4wDQYJKoZIhvcNAQEFBQAwXzELMAkGA1UEBhMCVVMx\n"
      "FzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMTcwNQYDVQQLEy5DbGFzcyAzIFB1YmxpYyBQcmltYXJ5\n"
      "IENlcnRpZmljYXRpb24gQXV0aG9yaXR5MB4XDTk2MDEyOTAwMDAwMFoXDTI4MDgwMjIzNTk1OVow\n"
      "XzELMAkGA1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMTcwNQYDVQQLEy5DbGFzcyAz\n"
      "IFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0aG9yaXR5MIGfMA0GCSqGSIb3DQEBAQUA\n"
      "A4GNADCBiQKBgQDJXFme8huKARS0EN8EQNvjV69qRUCPhAwL0TPZ2RHP7gJYHyX3KqhEBarsAx94\n"
      "f56TuZoAqiN91qyFomNFx3InzPRMxnVx0jnvT0Lwdd8KkMaOIG+YD/isI19wKTakyYbnsZogy1Ol\n"
      "hec9vn2a/iRFM9x2Fe0PonFkTGUugWhFpwIDAQABMA0GCSqGSIb3DQEBBQUAA4GBABByUqkFFBky\n"
      "CEHwxWsKzH4PIRnN5GfcX6kb5sroc50i2JhucwNhkcV8sEVAbkSdjbCxlnRhLQ2pRdKkkirWmnWX\n"
      "bj9T/UWZYB2oK0z5XqcJ2HUw19JlYD1n1khVdWk/kfVIC0dpImmClr7JyDiGSnoscxlIaU5rfGW/\n"
      "D/xwzoiQ\n"
      "-----END CERTIFICATE-----\n";

    m_networkSessionManager->addCaCertificate(cert_DigiCert_High_Assurance_EV_Root_CA);
    m_networkSessionManager->addCaCertificate(cert_DigiCert_Global_Root_CA);
    m_networkSessionManager->addCaCertificate(cert_Entrust_net_Secure_Server_CA);
    m_networkSessionManager->addCaCertificate(cert_Entrust_net_Premium_2048_Secure_Server_CA);
    m_networkSessionManager->addCaCertificate(cert_Verisign_Class_3_Public_Primary_Certification_Authority);
  });
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

  if (url.isValid() && setState(STATE_INITIALIZING)) {
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
  std::lock_guard<std::mutex> stateLock(m_stateMutex);

  if (state > m_state || (m_state == STATE_TERMINATED && state == STATE_INITIALIZING)) {
    m_state = state;
    return true;
  }
  return false;
}

bool NetworkSession::isState(State state) const
{
  std::lock_guard<std::mutex> stateLock(m_stateMutex);

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
