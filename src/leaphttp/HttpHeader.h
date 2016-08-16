// Copyright (C) 2012-2016 Leap Motion, Inc. All rights reserved.
#pragma once

#include "Cookie.h"

#include <algorithm>
#include <functional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#include <cctype>

class HttpHeader {
  public:
    HttpHeader(void) = default;
    HttpHeader(std::string userAgent);
    virtual ~HttpHeader();

    bool hasHeader(const std::string& key) const;
    std::string header(const std::string& key) const;
    std::set<std::string> headers() const;

    void setHeader(const std::string& key, const std::string& value, bool append = false);

    Cookie cookie(const std::string& name) const;
    bool hasCookie(const std::string& name) const;
    std::set<Cookie> cookies() const;

    void setCookie(const Cookie& cookie);
    void setCookies(const std::set<Cookie>& cookies);

    std::string userAgent() const;
    void setUserAgent(std::string userAgent);

  private:
    struct CaseInsensitve {
      // Hash
      std::size_t operator()(const std::string& key) const {
        std::string lower;
        lower.resize(key.size());
        std::transform(key.begin(), key.end(), lower.begin(), ::tolower);
        return std::hash<std::string>()(lower);
      };
      // Equal
      bool operator()(const std::string& a, const std::string& b) const {
        return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), [] (char ca, char cb) {
          return std::tolower(ca) == std::tolower(cb);;
        });
      };
    };
    std::unordered_map<std::string, std::vector<std::string>, CaseInsensitve, CaseInsensitve> m_headers;
    std::set<Cookie> m_cookies;
    std::string m_userAgent;
};
