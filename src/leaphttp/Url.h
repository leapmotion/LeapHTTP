// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#if !defined(__Url_h__)
#define __Url_h__

#include <string>

class Url {
  public:
    Url() { invalidate(); }
    Url(const std::string& url);
    ~Url();

    std::string scheme() const { return m_scheme; }
    std::string username() const { return m_username; }
    std::string password() const { return m_password; }
    std::string host() const { return m_host; }
    std::string port() const { return m_port; }
    std::string path() const { return m_path; }
    std::string query() const { return m_query; }
    std::string fragment() const { return m_fragment; }

    void setUrl(const std::string& url) { parse(url); }
    void setScheme(const std::string& scheme) { m_scheme = scheme; m_url.clear(); }
    void setUsername(const std::string& username) { m_username = username; m_url.clear(); }
    void setPassword(const std::string& password) { m_password = password; m_url.clear(); }
    void setHost(const std::string& host) { m_host = host; m_url.clear(); }
    void setPort(const std::string& port) { m_port = port; m_url.clear(); }
    void setPath(const std::string& path) { m_path = path; m_url.clear(); }
    void setQuery(const std::string& query) { m_query = query; m_url.clear(); }
    void setFragment(const std::string& fragment) { m_fragment = fragment; m_url.clear(); }

    bool isValid() const { return m_valid; }
    void invalidate();
    std::string toString() const;

    static std::string encode(const std::string& input);
    static std::string decode(const std::string& input);

  private:
    void parse(const std::string& url);

    mutable std::string m_url;
    std::string m_scheme;
    std::string m_username;
    std::string m_password;
    std::string m_host;
    std::string m_port;
    std::string m_path;
    std::string m_query;
    std::string m_fragment;

    bool m_valid;
};

#endif // __Url_h__
