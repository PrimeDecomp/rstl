#ifndef _CINPUTSTREAM
#define _CINPUTSTREAM

#include "rstl/RstlVersions.h"

#include "types.h"

#include "stddef.h"

#if RSTL_KYOTO_BLOCK_INPUT_STREAM
#ifndef __MWERKS__
#include <cstdlib>
#include <type_traits>
#endif
#else
#include "Kyoto/Basics/CBasics.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/pair.hpp"
#endif

class CInputStream;

template < typename T >
struct TType {};

#if RSTL_KYOTO_BLOCK_INPUT_STREAM
template < typename T >
T cinput_stream_helper(const TType< T >& type, CInputStream& in);
#endif

template < typename T >
inline TType< T > TGetType(const T&) {
  return TType< T >();
}

#if RSTL_KYOTO_BLOCK_INPUT_STREAM
// Prime's input stream reads blocks from a derived stream's virtual Read.
class CInputStream {
public:
  CInputStream(int len);
  CInputStream(const void* ptr, int len, bool owned);
  virtual ~CInputStream();
  virtual size_t Read(void* dest, size_t len) = 0;

  float ReadFloat();
  double ReadDouble();
  u64 ReadLongLong();
  uint ReadLong();
  ushort ReadShort();
  bool ReadBool();
  char ReadChar();
  uint ReadBits(uint len);
  size_t ReadBytes(void* dest, size_t len);
  void Get(void* dest, unsigned long len);

  template < typename T >
  T Get(const TType< T >& type = TType< T >()) {
    return cinput_stream_helper(TType< T >(), *this);
  }

  template < typename T >
  void Read(T& value) {
    value = T(*this);
  }

  bool ReadPackedBool() { return ReadBits(1) != 0; }

  unsigned char ReadUint8() {
    const unsigned char result = *mPtr++;
    return result;
  }
  char ReadInt8() { return static_cast< char >(ReadUint8()); }
  int ReadInt32() { return Get< int >(); }
  ushort ReadUint16() { return Get< ushort >(); }
  short ReadInt16() { return Get< short >(); }

  uint GetBlockOffset() const { return mBlockOffset; }
  const uint GetReadPosition() const { return mReadPosition; }

private:
  bool GrabAnotherBlock();
  bool InternalReadNext();

  uint mBlockOffset;
  uint mBlockLen;
  uint mLen;
  uchar* mPtr;
  bool mOwned;
  uint mReadPosition;
  uint mBitWord;
  uint mBitOffset;
};

DECLARE_FULL_SIZE_FOR(CInputStream, 0x24)
CHECK_SIZEOF(CInputStream, CInputStream_FULL_SIZE)

template < typename T >
inline T cinput_stream_helper(const TType< T >& type, CInputStream& in) {
#ifndef __MWERKS__
  if constexpr (!std::is_constructible_v< T, CInputStream& >) {
    abort();
  } else
#endif
  {
    return T(in);
  }
}
template <>
inline bool cinput_stream_helper(const TType< bool >& type, CInputStream& in) {
  return in.ReadBool();
}
template <>
inline char cinput_stream_helper(const TType< char >& type, CInputStream& in) {
  return in.ReadChar();
}

template <>
inline unsigned char cinput_stream_helper(const TType< unsigned char >& type, CInputStream& in) {
  return in.ReadChar();
}

template <>
inline signed char cinput_stream_helper(const TType< signed char >& type, CInputStream& in) {
  return in.ReadChar();
}

template <>
inline int cinput_stream_helper(const TType< int >& type, CInputStream& in) {
  return in.ReadLong();
}
template <>
inline uint cinput_stream_helper(const TType< uint >& type, CInputStream& in) {
  return in.ReadLong();
}
template <>
inline unsigned long cinput_stream_helper(const TType< unsigned long >& type, CInputStream& in) {
  return in.ReadLong();
}
template <>
inline float cinput_stream_helper(const TType< float >& type, CInputStream& in) {
  return in.ReadFloat();
}
template <>
inline short cinput_stream_helper(const TType< short >& type, CInputStream& in) {
  return in.ReadShort();
}
template <>
inline ushort cinput_stream_helper(const TType< ushort >& type, CInputStream& in) {
  return in.ReadShort();
}

#include "rstl/pair.hpp"

#include "rstl/vector.hpp"

#include "rstl/reserved_vector.hpp"

#include "rstl/set.hpp"

#else
// Echoes' and Corruption's input stream reads directly from a memory buffer.
class CInputStream {
public:
  struct SBufferAndSize {
    const void* mBuffer;
    unsigned long mSize;

    SBufferAndSize(const void* buffer, unsigned long size) : mBuffer(buffer), mSize(size) {}
  };

  CInputStream(const void* ptr, unsigned long len);
  CInputStream(const void* ptr, unsigned long len, bool owned);
  CInputStream(const SBufferAndSize& buffer, bool owned);
  virtual ~CInputStream();

  float ReadFloat();
  size_t ReadBytes(void* dest, size_t len);
  void Get(void* dest, unsigned long len);
  const void* Get(unsigned long len);
  rstl::auto_ptr< uchar > ReleaseBuffer();

  template < typename T >
  T Get(const TType< T >& type = TType< T >());

  // Guessed name for the typed object read-into operation used by script properties.
  template < typename T >
  void Read(T& value) {
    value = T(*this);
  }

#if RSTL_KYOTO_INPUT_STREAM_TYPED_CURSOR
  int ReadInt32() {
    int* result = reinterpret_cast< int* >(mpBufferAt.p8);
    mpBufferAt.p8 += sizeof(int);
    return BE_SWAP(*result);
  }
  uint ReadUint32() { // Guessed name.
    uint* result = mpBufferAt.p32++;
    return BE_SWAP(*result);
  }
  u64 ReadInt64() {
    u64* result = reinterpret_cast< u64* >(mpBufferAt.p8);
    mpBufferAt.p8 = reinterpret_cast< uchar* >(result + 1);
    return BE_SWAP(*result);
  }
  u16 ReadUint16() { return BE_SWAP(*mpBufferAt.p16++); }
  short ReadInt16() { return static_cast< short >(ReadUint16()); }
  u8 ReadUint8() {
    const u8 result = *mpBufferAt.p8++;
    return result;
  }
  char ReadInt8() { return *mpBufferAt.p8++; }
  bool ReadBool() { return ReadUint8() != 0; }
#else
  int ReadInt32() {
    int* result = reinterpret_cast< int* >(mpBufferAt.p8);
    mpBufferAt.p8 += sizeof(int);
    return BE_SWAP(*result);
  }
  uint ReadUint32() { // Guessed name.
    uint* result = reinterpret_cast< uint* >(mpBufferAt.p8);
    mpBufferAt.p8 += sizeof(uint);
    return BE_SWAP(*result);
  }
  u64 ReadInt64() {
    u64* result = reinterpret_cast< u64* >(mpBufferAt.p8);
    mpBufferAt.p8 += sizeof(u64);
    return BE_SWAP(*result);
  }
  u16 ReadUint16() {
    u16* result = reinterpret_cast< u16* >(mpBufferAt.p8);
    mpBufferAt.p8 = reinterpret_cast< uchar* >(result + 1);
    return BE_SWAP(*result);
  }
  short ReadInt16() { return static_cast< short >(ReadUint16()); }
  u8 ReadUint8() {
    const u8 result = *mpBufferAt.p8++;
    return result;
  }
  char ReadInt8() { return static_cast< char >(ReadUint8()); }
  bool ReadBool() { return *mpBufferAt.p8++ != 0; }
#endif
  uint GetReadPosition() const { return mpBufferAt.p8 - mpBufferStart; }

protected:
  uchar* mpBufferStart;
  union {
    uchar* p8;
    char* pc8;   // Guessed name
    ushort* p16; // Guessed name
    uint* p32;   // Guessed name
  } mpBufferAt;
  unsigned long mBufferSize;
  bool mOwner;
};

DECLARE_FULL_SIZE_FOR(CInputStream, 0x14)
CHECK_SIZEOF(CInputStream, CInputStream_FULL_SIZE)

template < typename T >
inline T CInputStream::Get(const TType< T >& type) {
  return T(*this);
}

template <>
inline bool CInputStream::Get< bool >(const TType< bool >& type) {
  return ReadBool();
}

#if RSTL_KYOTO_INPUT_STREAM_TYPED_CURSOR
template <>
inline char CInputStream::Get< char >(const TType< char >& type) {
  return *mpBufferAt.pc8++;
}
#else
template <>
inline char CInputStream::Get< char >(const TType< char >& type) {
  return ReadInt8();
}
#endif

template <>
inline unsigned char CInputStream::Get< unsigned char >(const TType< unsigned char >& type) {
  return ReadUint8();
}

#if RSTL_KYOTO_INPUT_STREAM_TYPED_CURSOR
template <>
inline signed char CInputStream::Get< signed char >(const TType< signed char >& type) {
  TType< char > charType;
  return Get(charType);
}
#else
template <>
inline signed char CInputStream::Get< signed char >(const TType< signed char >& type) {
  return ReadInt8();
}
#endif

template <>
inline int CInputStream::Get< int >(const TType< int >& type) {
  return ReadInt32();
}

template <>
inline uint CInputStream::Get< uint >(const TType< uint >& type) {
  return ReadUint32();
}

#if RSTL_KYOTO_INPUT_STREAM_TYPED_CURSOR
template <>
inline unsigned long CInputStream::Get< unsigned long >(const TType< unsigned long >& type) {
  return ReadUint32();
}
#else
template <>
inline unsigned long CInputStream::Get< unsigned long >(const TType< unsigned long >& type) {
  return ReadInt32();
}
#endif

template <>
inline u64 CInputStream::Get< u64 >(const TType< u64 >& type) {
  return ReadInt64();
}

template <>
inline float CInputStream::Get< float >(const TType< float >& type) {
  return ReadFloat();
}

template <>
inline short CInputStream::Get< short >(const TType< short >& type) {
  return ReadInt16();
}

template <>
inline ushort CInputStream::Get< ushort >(const TType< ushort >& type) {
  return ReadUint16();
}

#include "rstl/vector.hpp"

#include "rstl/reserved_vector.hpp"

#include "rstl/red_black_tree.hpp"

#endif

#endif // _CINPUTSTREAM
