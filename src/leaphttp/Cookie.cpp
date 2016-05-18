// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#include "stdafx.h"
#include "Cookie.h"
#include <algorithm>

Cookie::Cookie(const std::string& name, const std::string& value) :
  m_name(name), m_isSecure(false), m_isHttpOnly(false)
{
  parse(value);
}

Cookie::~Cookie()
{
}

bool Cookie::isValid() const
{
  return (!m_name.empty() && !m_value.empty());
}

std::string Cookie::name() const
{
  return m_name;
}

void Cookie::setName(const std::string& name)
{
  m_name = name;
}

std::string Cookie::value() const
{
  return m_value;
}

void Cookie::setValue(const std::string& value)
{
  parse(value);
}

std::string Cookie::path() const
{
  return m_path;
}

void Cookie::setPath(const std::string& path)
{
  m_path = path;
}

std::string Cookie::domain() const
{
  return m_domain;
}

bool Cookie::isSecure() const
{
  return m_isSecure;
}

bool Cookie::isHttpOnly() const
{
  return m_isHttpOnly;
}

std::string Cookie::toString() const
{
  if (!isValid()) {
    return std::string();
  }
  return (m_name + "=" + m_value);
}

void Cookie::parse(const std::string& value)
{
  std::string::size_type lastPos = value.find_first_of(';');

  m_value = value.substr(0, lastPos);

  // Find attributes
  while (lastPos != std::string::npos) {
    // Skip spaces
    std::string::size_type skipSpacesPos = value.find_first_not_of(' ', ++lastPos);
    if (skipSpacesPos != std::string::npos) {
      lastPos = skipSpacesPos;
    }

    // Find attribute
    std::string::size_type pos = value.find_first_of(';', lastPos);
    std::string attribute = value.substr(lastPos, pos - lastPos);

    // Find name-value of attribute
    std::string::size_type equalPos = attribute.find_first_of('=');
    std::string attrName = attribute.substr(0, equalPos);
    std::string attrValue;
    if (equalPos != std::string::npos) {
      attrValue = attribute.substr(++equalPos);
    }
    std::transform(attrName.begin(), attrName.end(), attrName.begin(), ::tolower);

    // Set the appropriate Cookie attribute
    if (attrName == "path") {
      m_path = attrValue;
    } else if (attrName == "domain") {
      m_domain = attrValue;
    } else if (attrName == "expires") {
      // FIXME
    } else if (attrName == "max-age") {
      // FIXME
    } else if (attrName == "secure") {
      m_isSecure = true;
    } else if (attrName == "httponly") {
      m_isHttpOnly = true;
    }
    lastPos = pos;
  }
}
