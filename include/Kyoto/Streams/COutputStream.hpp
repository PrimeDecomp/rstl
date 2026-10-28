#ifndef _COUTPUTSTREAM
#define _COUTPUTSTREAM

#include "rstl/RstlVersions.h"

#include "Kyoto/Basics/CBasics.hpp"
#include "types.h"

#include "rstl/pair.hpp"
#include "stddef.h"

class COutputStream;

template < typename T >
void coutput_stream_helper(const T& t, COutputStream& out);

class COutputStream {
public:
  explicit COutputStream(int len);
  virtual ~COutputStream();
  virtual void Write(const void* ptr, size_t len) = 0;
#if RSTL_KYOTO_OUTPUT_SHIFT_REGISTER
  void WriteBits(uint val, uint bitCount);
#endif

  void DoPut(const void* ptr, size_t len);
  void DoFlush();
#if RSTL_KYOTO_OUTPUT_SHIFT_REGISTER
  void Flush();
  void FlushShiftRegister();
  void Put(const void* ptr, size_t len) {
    FlushShiftRegister();
    DoPut(ptr, len);
  }
#else
  void Flush() { DoFlush(); }
  void Put(const void* ptr, size_t len) { DoPut(ptr, len); }
#endif

  template < typename T >
  void Put(const T& t) {
    coutput_stream_helper(t, *this);
  }

  void WriteInt8(const signed char t) { Put(t); }
  void WriteUint8(const uchar t) { Put(t); }

  void WriteInt16(const short t) { Put(t); }
  void WriteUint16(const ushort t) { Put(t); }

  void WriteReal32(const float t) { Put(t); }

  void WriteUint32(const uint t) { Put(t); }
  void WriteInt32(const int t) { Put(t); }

  void WriteShort(const short t) {
    const short value = CBasics::SwapBytes(t);
    Put(&value, sizeof(value));
  }
  void WriteLong(const uint t) {
    const uint value = CBasics::SwapBytes(t);
    Put(&value, sizeof(value));
  }

  void WriteBool(const bool b) {
#if RSTL_KYOTO_OUTPUT_SHIFT_REGISTER
    FlushShiftRegister();
#endif
    if (mUnwrittenLen >= mBufLen) {
      DoFlush();
    }
    ++mNumWrites;
    *(static_cast< uchar* >(mBufPtr) + mUnwrittenLen++) = b ? 1 : 0;
  }
  void WriteChar(const uchar c) {
#if RSTL_KYOTO_OUTPUT_SHIFT_REGISTER
    FlushShiftRegister();
#endif
    if (mUnwrittenLen >= mBufLen) {
      DoFlush();
    }
    ++mNumWrites;
    *(reinterpret_cast< uchar* >(mBufPtr) + mUnwrittenLen++) = c;
  }

  uint GetWrittenBytes() const { return mNumWrites; }

private:
  uint mUnwrittenLen;
  uint mBufLen;
  void* mBufPtr;
  uint mNumWrites;
#if RSTL_KYOTO_OUTPUT_SHIFT_REGISTER
  uint mShiftRegister;
  volatile uint mShiftRegisterOffset;
#endif
#if RSTL_KYOTO_OUTPUT_SCRATCH_BUFFER
  uchar mScratch[96];
#endif
};

#if RSTL_KYOTO_OUTPUT_SCRATCH_BUFFER
DECLARE_FULL_SIZE_FOR(COutputStream, RSTL_KYOTO_OUTPUT_SHIFT_REGISTER ? 0x7C : 0x74)
#else
DECLARE_FULL_SIZE_FOR(COutputStream, 0x1C)
#endif
CHECK_SIZEOF(COutputStream, COutputStream_FULL_SIZE)

template < typename T >
inline void coutput_stream_helper(const T& t, COutputStream& out) {
  t.PutTo(out);
}

#if RSTL_KYOTO_OUTPUT_PAIR_HELPER
template < typename L, typename R >
inline void coutput_stream_helper(const rstl::pair< L, R >& t, COutputStream& out) {
  out.Put(t.first);
  out.Put(t.second);
}
#endif

template <>
inline void coutput_stream_helper(const float& t, COutputStream& out) {
  const float value = t;
  out.WriteLong(*reinterpret_cast< const uint* >(&value));
}

template <>
inline void coutput_stream_helper(const signed char& t, COutputStream& out) {
  out.WriteChar(t);
}

template <>
inline void coutput_stream_helper(const uchar& t, COutputStream& out) {
  out.WriteChar(t);
}

template <>
inline void coutput_stream_helper(const short& t, COutputStream& out) {
  out.WriteShort(t);
}

template <>
inline void coutput_stream_helper(const ushort& t, COutputStream& out) {
  out.WriteShort(t);
}

template <>
inline void coutput_stream_helper(const int& t, COutputStream& out) {
  out.WriteLong(t);
}

template <>
inline void coutput_stream_helper(const uint& t, COutputStream& out) {
  out.WriteLong(t);
}

template <>
inline void coutput_stream_helper(const bool& t, COutputStream& out) {
  out.WriteChar(static_cast< u8 >(t));
}

namespace rstl {
  
template < typename L, typename R >
inline void pair< L, R >::PutTo(COutputStream& out) const {
  out.Put(first);
  out.Put(second);
}

}

#endif // _COUTPUTSTREAM
