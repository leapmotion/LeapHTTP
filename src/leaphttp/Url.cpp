// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#include "stdafx.h"
#include "Url.h"
#include <stdexcept>
#include <sstream>

#include <iostream>
#include <iomanip>

Url::Url(const std::string& url) : m_valid(false)
{
  parse(url);
}

Url::~Url()
{
}

//
// This is a poor man's URL parser. It does very minimal validation and doesn't
// properly handle reserved characters. However, for our needs, this is
// currently sufficient for handling our well-formed URLs.
//
void Url::parse(const std::string& url)
{
  const char* p = url.c_str();

  std::string temp;
  temp.reserve(url.size());

  try {
    int state = 0;
    char c;

    // Use a state machine to decide how to process the next character
    while ((c = *p++) != '\0') {
      switch (state) {
      case 0: // Scheme
        if (c >= 'A' && c <= 'Z') {
          temp.push_back(c - 'A' + 'a');
        } else if (c >= 'a' && c <= 'z') {
          temp.push_back(c);
        } else if (temp.empty()) {
          throw std::runtime_error("Scheme must begin with letter");
        } else if (c == '+' || c == '.' || c == '-' || (c >= '0' && c <= '9')) {
          temp.push_back(c);
        } else if (c == ':') {
          m_scheme = temp;
          temp.clear();
          state++;
        } else {
          throw std::runtime_error("Scheme encountered invalid character");
        }
        break;
      case 1: // First forward slash
        if (c == '/') {
          state++;
        } else { // No authority, skip right to path
          temp.push_back(c);
          state = 7; // Path
        }
        break;
      case 2: // Second forward slash
        if (c == '/') {
          state++;
        } else { // No authority we're already parsing the path
          temp.push_back('/');
          temp.push_back(c);
          state = 7; // Path
        }
        break;
      case 3: // Username (or Hostname)
        if (c == ':') { // Splitting up either username:password or hostname:port
          m_host = temp; // Assume it is hostname for now (correct as needed)
          temp.clear();
          state++;
        } else if (c == '@') {
          m_username = temp;
          temp.clear();
          state = 5; // Hostname
        } else if (c == '/') {
          m_host = temp;
          temp.clear();
          temp.push_back(c);
          state = 7; // Path
        } else if (c == '?' || c == '#') {
          m_host = temp;
          temp.clear();
          m_path = '/';
          state = c == '?' ? 8 : 9; // Query or Fragment
        } else {
          temp.push_back(c);
          if (c == '[') {
            if (temp.size() == 1) { // This is an IPv6 hostname
              state = 5; // Hostname
            } else {
              throw std::runtime_error("Encountered reserved character while parsing authority");
            }
          } else if (c == ']' && (*p != ':' || *p != '/' || *p != '?' || *p != '#' || *p != '\0')) {
            throw std::runtime_error("Encountered reserved character while parsing authority");
          }
        }
        break;
      case 4: // Password (or Port)
        if (c == ':') {
          throw std::runtime_error("Encountered reserved character while parsing authority");
        } else if (c == '@') {
          m_username = m_host; // We made the wrong assumption about the hostname (it was really the username)
          m_host.clear();
          m_password = temp;
          temp.clear();
          state++;
        } else if (c == '/') {
          m_port = temp;
          temp.clear();
          temp.push_back(c);
          state = 7; // Path
        } else if (c == '?' || c == '#') {
          m_port = temp;
          temp.clear();
          m_path = '/';
          state = c == '?' ? 8 : 9; // Query or Fragment
        } else {
          temp.push_back(c);
        }
        break;
      case 5: // Hostname
        if (c == ':') { // Splitting up either hostname:port
          if (!temp.empty() && temp[0] == '[' && temp[temp.size() - 1] != ']') {
            temp.push_back(c); // We're in an IPv6 hostname, not splitting up hostname and port
          } else {
            m_host = temp;
            temp.clear();
            state++;
          }
        } else if (c == '@') {
          throw std::runtime_error("Encountered reserved character while parsing hostname");
        } else if (c == '/') {
          m_host = temp;
          temp.clear();
          temp.push_back(c);
          state = 7; // Path
        } else if (c == '?' || c == '#') {
          m_host = temp;
          temp.clear();
          m_path = '/';
          state = c == '?' ? 8 : 9; // Query or Fragment
        } else {
          temp.push_back(c);
        }
        break;
      case 6: // Port
        if (c == ':' || c == '@') {
          throw std::runtime_error("Encountered reserved character while parsing port");
        } else if (c == '/') {
          m_port = temp;
          temp.clear();
          temp.push_back(c);
          state = 7; // Path
        } else if (c == '?' || c == '#') {
          m_port = temp;
          temp.clear();
          m_path = '/';
          state = c == '?' ? 8 : 9; // Query or Fragment
        } else {
          temp.push_back(c);
        }
        break;
      case 7: // Path
        if (c == '?' || c == '#') {
          m_path = temp;
          temp.clear();
          state = c == '?' ? 8 : 9; // Query or Fragment
        } else {
          temp.push_back(c);
        }
        break;
      case 8: // Query
        if (c == '#') {
          m_query = temp;
          temp.clear();
          state++;
        } else {
          temp.push_back(c);
        }
        break;
      case 9: // Fragment
        temp.push_back(c);
        break;
      default:
        break;
      }
    }
    // Assign the remaining content to the component we were working on before we reached the end
    switch (state) {
    case 3:
    case 5:
      m_host = temp;
      m_path = '/';
      break;
    case 4:
    case 6:
      m_port = temp;
      m_path = '/';
      break;
    case 7:
      m_path = temp;
      break;
    case 8:
      m_query = temp;
      break;
    case 9:
      m_fragment = temp;
      break;
    default:
      throw std::runtime_error("Invalid URL");
      break;
    }
    // Verify that the port is a valid number
    if (!m_port.empty()) {
      unsigned long port = 0;

      for (const char* p = m_port.c_str(); *p != '\0'; p++) {
        if (*p >= '0' && *p <= '9') {
          port = port*10 + (*p - '0');
        } else {
          throw std::runtime_error("Encountered invalid character while parsing port");
        }
      }
      // Remove the port from the URL when using well-known port numbers
      if ((port ==  80 && m_scheme == "http") ||
          (port == 443 && m_scheme == "https") ||
          (port ==  21 && m_scheme == "ftp") ||
          (port == 990 && m_scheme == "ftps") ||
          (port ==  23 && m_scheme == "telnet")) {
        m_port.clear();
      }
    }
    m_valid = true;
  } catch (...) {
    invalidate();
  }
}

std::string Url::toString() const
{
  if (m_valid && m_url.empty()) {
    std::ostringstream oss;

    oss << m_scheme << ':';
    if (!m_path.empty() && m_path[0] == '/') {
      oss << "//";
      if (!m_username.empty()) {
        oss << m_username;
      }
      if (!m_password.empty()) {
        oss << ':' << m_password << '@';
      } else if (!m_username.empty()) {
        oss << '@';
      }
      oss << m_host;
      if (!m_port.empty()) {
        oss << ':' << m_port;
      }
    }
    oss << m_path;
    if (!m_query.empty()) {
      oss << '?' << m_query;
    }
    if (!m_fragment.empty()) {
      oss << '#' << m_fragment;
    }
    m_url = oss.str();
  }
  return m_url;
}

std::string Url::encode(const std::string& input)
{
  std::ostringstream oss;

  for (const char* bytes = input.c_str(); *bytes != '\0'; bytes++) {
    char c = *bytes;
    if ((c >= 'A' && c <= 'Z') ||
        (c >= 'a' && c <= 'z') ||
        (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
      oss << c;
    } else {
      unsigned char uc = static_cast<unsigned char>(c);
      oss << '%' << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(uc);
    }
  }
  return oss.str();
}

std::string Url::decode(const std::string& input)
{
  std::ostringstream oss;

  for (const char* bytes = input.c_str(); *bytes != '\0'; bytes++) {
    if (*bytes == '%') {
      unsigned char value = 0;
      char hi = *++bytes;
      if (hi == '\0') {
        break;
      }
      char lo = *++bytes;
      if (lo == '\0') {
        break;
      }
      if (hi >= '0' && hi <= '9') {
        value = (hi - '0')*16;
      } else if (hi >= 'A' && hi <= 'F') {
        value = (hi - 'A' + 10)*16;
      } else if (hi >= 'a' && hi <= 'f') {
        value = (hi - 'a' + 10)*16;
      } else {
        break;
      }
      if (lo >= '0' && lo <= '9') {
        value += lo - '0';
      } else if (lo >= 'A' && lo <= 'F') {
        value += lo - 'A' + 10;
      } else if (lo >= 'a' && lo <= 'f') {
        value += lo - 'a' + 10;
      } else {
        break;
      }
      oss << static_cast<char>(value);
    } else {
      oss << *bytes;
    }
  }
  return oss.str();
}

void Url::invalidate() {
  m_scheme.clear();
  m_username.clear();
  m_password.clear();
  m_host.clear();
  m_port.clear();
  m_query.clear();
  m_fragment.clear();
  m_url.clear();
  m_valid = false;
}
