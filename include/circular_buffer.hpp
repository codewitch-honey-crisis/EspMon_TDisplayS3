#pragma once
#include <stdlib.h>
// a circular buffer
template <typename T, size_t Capacity>
class circular_buffer {
    T m_data[Capacity];
    size_t m_head;
    size_t m_tail;
    bool m_full;
    void advance() {
        if (m_full) {
            if (++(m_tail) == capacity) {
                m_tail = 0;
            }
        }

        if (++(m_head) == capacity) {
            m_head = 0;
        }
        m_full = (m_head == m_tail);
    }
    void retreat() {
        m_full = false;
        if (++(m_tail) == capacity) {
            m_tail = 0;
        }
    }

   public:
    using type = circular_buffer;
    using value_type = T;
    constexpr static const size_t capacity = Capacity;

    inline circular_buffer() {
        clear();
    }
    inline bool empty() const {
        return (!m_full && (m_head == m_tail));
    }
    inline bool full() const {
        return m_full;
    }
    size_t size() const {
        size_t result = capacity;
        if (!m_full) {
            if (m_head >= m_tail) {
                result = (m_head - m_tail);
            } else {
                result = (capacity + m_head - m_tail);
            }
        }
        return result;
    }
    inline void clear() {
        m_head = 0;
        m_tail = 0;
        m_full = false;
    }
    void put(const value_type& value) {
        m_data[m_head] = value;
        advance();
    }
    const value_type* peek(size_t index = 0) const {
        if (!empty() && index<size()) {
            return m_data + ((m_tail+index)%capacity);
        }
        return nullptr;
    }
    bool get(value_type* out_value) {
        if (!empty()) {
            if (out_value != nullptr) {
                *out_value = m_data[m_tail];
            }
            retreat();
            return true;
        }
        return false;
    }
    
};