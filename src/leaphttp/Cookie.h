// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#if !defined(__Cookie_h__)
#define __Cookie_h__

#include <string>

class Cookie {
  public:
    Cookie(const std::string& name = std::string(), const std::string& value = std::string());
    virtual ~Cookie();

    bool isValid() const;

    std::string name() const;
    void setName(const std::string& name);

    std::string value() const;
    void setValue(const std::string& value);

    std::string path() const;
    void setPath(const std::string& path);

    std::string domain() const;
    bool isSecure() const;
    bool isHttpOnly() const;

    std::string toString() const;

    inline bool operator<(const Cookie& other) const { return m_name < other.m_name; }

  protected:
    void parse(const std::string& value);

  private:
    std::string m_name;
    std::string m_value;
    std::string m_path;
    std::string m_domain;
    bool m_isSecure;
    bool m_isHttpOnly;
};

#endif // __Cookie_h__

