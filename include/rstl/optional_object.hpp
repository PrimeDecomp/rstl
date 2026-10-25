#ifndef _RSTL_OPTIONAL_OBJECT
#define _RSTL_OPTIONAL_OBJECT

#include "types.h"

#include "rstl/RstlVersions.h"
#include "rstl/construct.hpp"

namespace rstl {
struct optional_object_null {};

template < typename T >
class optional_object {
public:
  optional_object() : m_valid(false) {}
  optional_object(optional_object_null) : m_valid(false) {}
  optional_object(const T& item) : m_valid(true) { rstl::construct< T >(m_data, item); }
#if RSTL_OPTIONAL_COPY_READS_VALID_CALL
  optional_object(const optional_object& other) : m_valid(other.valid()) {
#else
  optional_object(const optional_object& other) : m_valid(other.m_valid) {
#endif
    if (other.valid()) {
      construct< T >(m_data, other.data());
    }
  }
  ~optional_object() {
#if RSTL_OPTIONAL_DTOR_DESTROYS
    // Makes ~CScriptHudMemo match (instead of clear())
    if (m_valid) {
      rstl::destroy(get_ptr());
    }
#else
    clear();
#endif
  }

  optional_object& operator=(const optional_object& other) {
    if (this == &other) {
      return *this;
    }
    if (other.valid()) {
#if RSTL_OPTIONAL_ASSIGN_INLINE
      if (!m_valid) {
        construct< T >(m_data, other.data());
        m_valid = true;
      } else {
        *get_ptr() = other.data();
      }
#else
      assign(other.data());
#endif
    } else {
      clear();
    }
    return *this;
  }
  optional_object& operator=(const T& item) {
    assign(item);
    return *this;
  }

  T& data() { return *get_ptr(); }
  const T& data() const { return *get_ptr(); }
  template < typename A1, typename A2, typename A3, typename A4 >
  T& emplace(const A1& a1, const A2& a2, const A3& a3, const A4& a4) {
    clear();
    new (m_data) T(a1, a2, a3, a4);
    m_valid = true;
    return data();
  }
  T* prepare_emplace() {
    if (m_valid) {
      rstl::destroy(get_ptr());
    }
    m_valid = true;
    return get_ptr();
  }
#if RSTL_DESTROY_CONST_PARAM
  T* const get_ptr() { return reinterpret_cast< T* >(m_data); }
#else
  T* get_ptr() { return reinterpret_cast< T* >(m_data); }
#endif
  const T* get_ptr() const { return reinterpret_cast< const T* >(m_data); }
  const bool valid() const { return m_valid; }
  operator bool() const { return m_valid; } // replace with valid()?
  void clear() {
    if (m_valid) {
      rstl::destroy(get_ptr());
    }
    m_valid = false;
  }

  T& operator*() { return data(); }
  T* operator->() { return &data(); }

  const T& operator*() const { return data(); }
  const T* operator->() const { return &data(); }

private:
#if RSTL_OPTIONAL_OBJECT_PLAIN_STORAGE
  uchar m_data[sizeof(T)];
  bool m_valid ATTRIBUTE_ALIGN(4);
#elif RSTL_OPTIONAL_OBJECT_ALIGNED_STORAGE
  ALIGNAS(T) uchar m_data[sizeof(T)];
  ATTRIBUTE_ALIGN_DECL(4, bool m_valid);
#else
#if defined(__MWERKS__) || CLANGD
  // MWCC 1.3.2 does not support template-dependent alignment, so it's assumed
  // that Retro used a uint array here to get 4-byte alignment. Clever!
  uint m_data[(sizeof(T) + sizeof(uint) - 1) / sizeof(uint)];
#else
  ALIGNAS(T) uchar m_data[sizeof(T)];
#endif
  bool m_valid;
#endif

  void assign(const T& item) {
    if (!m_valid) {
      construct< T >(m_data, item);
      m_valid = true;
      return;
    }
    *get_ptr() = item;
  }
};
} // namespace rstl

#endif // _RSTL_OPTIONAL_OBJECT
