// Copyright (C) 2012-2018 Leap Motion, Inc. All rights reserved.
#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <cstring>

//
// Single Producer and Single Consumer of this circular buffer stream
//
template<typename T>
class CircularBufferStream {
  public:
    CircularBufferStream(size_t size) : m_array(new T[size]), m_end(m_array.get() + size), m_head(m_array.get()), m_tail(m_array.get()),
                                        m_capacity(size), m_count(0), m_isClosed(false) {
      // Only use types that may be safely memcpy'ed (e.g., char)
      static_assert(std::is_pod<T>::value, "T must be a POD");
    }
    ~CircularBufferStream() {
      close();
    }

    void open() {
      std::lock_guard<std::mutex> lock(m_mutex);
      if (isClosed()) {
        m_head = m_array.get();
        m_tail = m_array.get();
        m_count = 0;
        m_isClosed = false;
      }
    }

    void close() {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_isClosed = true;
      m_condition.notify_all();
    }

    size_t write(const T* items, const size_t size) {
      if (size == 0) {
        return 0;
      }
      std::unique_lock<std::mutex> lock(m_mutex);
      if (isClosed()) {
        return 0;
      }
      T* tail = m_tail;
      T* head = m_head;
      size_t count = m_count;
      lock.unlock();

      size_t remainingSize = size;

      // Loop until we have written all of our data
      while (remainingSize > 0) {
        size_t currentSize = remainingSize;
        size_t totalWritten = 0;

        if (count != m_capacity) { // not Full?
          // We cannot write more than what fits into our buffer, so set a top limit
          if (currentSize > m_capacity) {
            currentSize = m_capacity;
          }
          if (tail >= head) {
            if (tail + currentSize >= m_end) {
              const size_t partial = m_end - tail;

              std::memcpy(tail, items, partial*sizeof(T));
              currentSize -= partial;
              totalWritten += partial;
              if (currentSize > 0) {
                size_t available = head - m_array.get();

                if (currentSize > available) {
                  currentSize = available;
                }
                std::memcpy(m_array.get(), items + partial, currentSize*sizeof(T));
                totalWritten += currentSize;
              }
              tail = m_array.get() + currentSize;
            } else {
              std::memcpy(tail, items, currentSize*sizeof(T));
              totalWritten += currentSize;
              tail += currentSize;
            }
          } else {
            size_t available = head - tail;

            if (currentSize > available) {
              currentSize = available;
            }
            std::memcpy(tail, items, currentSize*sizeof(T));
            totalWritten += currentSize;
            tail += currentSize;
          }
        }

        lock.lock();
        if (isClosed()) {
          // The stream has been closed, do not write any more data
          lock.unlock();
          break;
        }
        m_count += totalWritten; // Update the count
        m_tail = tail; // Update our new position
        head = m_head; // Capture the new head
        count = m_count; // Capture the new count
        if (totalWritten > 0 || isFull()) {
          items += totalWritten;
          remainingSize -= totalWritten;
          m_condition.notify_all(); // Let others know that we have written data
        }
        if (remainingSize > 0 && isFull()) {
          m_condition.wait(lock); // Wait for some data
          head = m_head; // Capture the new head (again)
          count = m_count; // Capture the new count (again)
        }
        lock.unlock();
      }

      return size - remainingSize;
    }

    size_t read(T* items, size_t size) {
      if (size == 0) {
        return 0;
      }
      std::unique_lock<std::mutex> lock(m_mutex);
      if (isClosed() && isEmpty()) {
        return 0;
      }
      T* head = m_head;
      T* tail = m_tail;
      size_t count = m_count;
      lock.unlock();

      size_t remainingSize = size;

      // Loop until we have read all of the data
      while (remainingSize > 0) {
        size_t currentSize = remainingSize;
        size_t totalRead = 0;

        if (count != 0) { // not Empty?
          // We cannot read more than what fits into our buffer, so set an upper limit
          if (currentSize > m_capacity) {
            currentSize = m_capacity;
          }
          if (head >= tail) {
            if (head + currentSize >= m_end) {
              const size_t partial = m_end - head;

              std::memcpy(items, head, partial*sizeof(T));
              currentSize -= partial;
              totalRead += partial;
              if (currentSize > 0) {
                size_t available = tail - m_array.get();

                if (currentSize > available) {
                  currentSize = available;
                }
                std::memcpy(items + partial, m_array.get(), currentSize*sizeof(T));
                totalRead += currentSize;
              }
              head = m_array.get() + currentSize;
            } else {
              std::memcpy(items, head, currentSize*sizeof(T));
              totalRead += currentSize;
              head += currentSize;
            }
          } else {
            const size_t available = tail - head;

            if (currentSize > available) {
              currentSize = available;
            }
            std::memcpy(items, head, currentSize*sizeof(T));
            totalRead += currentSize;
            head += currentSize;
          }
        }

        lock.lock();
        m_count -= totalRead; // Update the count
        m_head = head; // Update our new position
        tail = m_tail; // Capture the new tail
        count = m_count; // Capture the new count
        if (totalRead > 0 || isEmpty()) {
          items += totalRead;
          remainingSize -= totalRead;
          m_condition.notify_all(); // Let others know that we have read data
        }
        if (isEmpty()) { // Empty?
          if (isClosed()) {
            // No data left and the stream is closed
            lock.unlock();
            break;
          } else if (remainingSize > 0) {
            m_condition.wait(lock); // Wait for some data
            tail = m_tail; // Capture the new tail (again)
            count = m_count; // Capture the new count (again)
          }
        }
        lock.unlock();
      }

      return size - remainingSize;
    }
    bool isClosed() const { return m_isClosed; }
    size_t size() const { return m_count; }
    size_t capacity() const { return m_capacity; }

  private:
    bool isEmpty() const { return (m_count == 0); }
    bool isFull() const { return (m_count == m_capacity); }

    mutable std::mutex m_mutex;
    std::condition_variable m_condition;
    const std::unique_ptr<T> m_array;
    T* const m_end;
    T* m_head;
    T* m_tail;
    const size_t m_capacity;
    size_t m_count;
    bool m_isClosed;
};
