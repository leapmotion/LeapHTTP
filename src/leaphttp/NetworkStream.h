// Copyright (C) 2012-2016 Leap Motion, Inc. All rights reserved.
#if !defined(__NetworkStream_h__)
#define __NetworkStream_h__

#include "CircularBufferStream.h"

#include <type_traits>
#include <iostream>

template <class cT, class traits = std::char_traits<cT>>
class basic_networkstreambuf : public std::basic_streambuf<cT, traits> {
  public:
    basic_networkstreambuf(CircularBufferStream<cT>& cbs) : std::basic_streambuf<cT, traits>(), m_cbs(cbs), m_offset(0), m_size(0) {}
    ~basic_networkstreambuf() {}

    //
    // Input functions (get):
    //
    virtual std::streamsize showmanyc() override {
      size_t size = (m_size - m_offset) + m_cbs.size();
      if (size == 0) {
        // If the buffer is closed, we need to check again to see if data
        // became available since we just checked:
        if (m_cbs.isClosed() && m_cbs.size() == 0) {
          return -1;
        }
      }
      return size;
    }

    virtual typename traits::int_type underflow() override {
      // Since the circular buffer stream will block if we request more data
      // than is available, we will only load what is available. If nothing is
      // available, we will try to load a single byte. We will then block
      // until a byte is available or the stream has been closed.
      size_t size = m_size - m_offset;
      if (size == 0) {
        size = m_cbs.size();
        if (size > MAX_BUFFER_SIZE) {
          size = MAX_BUFFER_SIZE;
        }
        size = m_cbs.read(m_buffer, size == 0 ? 1 : size);
        m_offset = 0;
        m_size = size;
        if (size == 0) {
          return traits::eof();
        }
      }
      // When converting to an int type, make sure that we treat source values as unsigned (as -1 means EOF)
      return static_cast<typename std::make_unsigned<cT>::type>(m_buffer[m_offset]);
    }

    virtual typename traits::int_type uflow() override {
      size_t size = m_size - m_offset;
      if (size == 0) {
        size = m_cbs.size();
        if (size > MAX_BUFFER_SIZE) {
          size = MAX_BUFFER_SIZE;
        }
        size = m_cbs.read(m_buffer, size == 0 ? 1 : size);
        m_offset = 0;
        m_size = size;
        if (size == 0) {
          return traits::eof();
        }
      }
      // When converting to an int type, make sure that we treat source values as unsigned (as -1 means EOF)
      return static_cast<typename std::make_unsigned<cT>::type>(m_buffer[m_offset++]);
    }

    virtual std::streamsize xsgetn(cT* s, std::streamsize count) override {
      // See underflow() comments
      std::streamsize remaining = count;
      while (remaining > 0) {
        size_t size = m_size - m_offset;
        if (size == 0) {
          size = m_cbs.size();
          if (size > MAX_BUFFER_SIZE) {
            size = MAX_BUFFER_SIZE;
          }
          size = m_cbs.read(m_buffer, size == 0 ? 1 : size);
          m_offset = 0;
          m_size = size;
          if (size == 0) {
            break;
          }
        }
        if (size > static_cast<size_t>(remaining)) {
          size = static_cast<size_t>(remaining);
        }
        std::memcpy(s, &m_buffer[m_offset], size*sizeof(cT));
        s += size;
        m_offset += size;
        remaining -= size;
      }
      return count - remaining;
    }

    //
    // Output functions (put):
    //
    virtual int sync() override {
      size_t size = m_cbs.write(m_buffer, m_offset);
      if (size != m_offset) {
        return -1;
      }
      m_offset = 0;
      return 0;
    }

    virtual typename traits::int_type overflow(typename traits::int_type c) override {
      if (m_offset == MAX_BUFFER_SIZE && sync() == -1) {
        return traits::eof();
      }
      m_buffer[m_offset++] = c;
      return traits::not_eof(c);
    }

    virtual std::streamsize xsputn(const cT* s, std::streamsize count) override {
      std::streamsize remaining = count;
      while (remaining > 0) {
        if (m_offset == MAX_BUFFER_SIZE && sync() == -1) {
          break;
        }
        size_t size = MAX_BUFFER_SIZE - m_offset;
        if (size > static_cast<size_t>(remaining)) {
          size = static_cast<size_t>(remaining);
        }
        std::memcpy(&m_buffer[m_offset], s, size*sizeof(cT));
        s += size;
        m_offset += size;
        remaining -= size;
      }
      return count - remaining;
    }

  protected:
    enum { MAX_BUFFER_SIZE = 1024 };
    cT m_buffer[MAX_BUFFER_SIZE];
    size_t m_offset;
    size_t m_size;
    CircularBufferStream<cT>& m_cbs;
};

template <class cT, class traits = std::char_traits<cT>>
class basic_inetworkstream : public std::basic_istream<cT, traits> {
  public:
    basic_inetworkstream(CircularBufferStream<cT>& cbs) :
      std::basic_ios<cT, traits>(),
      std::basic_istream<cT, traits>(nullptr),
      m_sbuf(cbs)
    {
      this->init(&m_sbuf);
    }

  protected:
    basic_networkstreambuf<cT, traits> m_sbuf;
};

template <class cT, class traits = std::char_traits<cT>>
class basic_onetworkstream : public std::basic_ostream<cT, traits> {
  public:
    basic_onetworkstream(CircularBufferStream<cT>& cbs) :
      std::basic_ios<cT, traits>(),
      std::basic_ostream<cT, traits>(nullptr),
      m_sbuf(cbs)
    {
      this->init(&m_sbuf);
    }

  protected:
    basic_networkstreambuf<cT, traits> m_sbuf;
};

typedef basic_onetworkstream<char> OutputNetworkStream;
typedef basic_inetworkstream<char> InputNetworkStream;

#endif // __NetworkStream_h__
