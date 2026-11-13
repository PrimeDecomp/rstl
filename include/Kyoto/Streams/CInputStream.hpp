#ifndef _CINPUTSTREAM
#define _CINPUTSTREAM

#include "rstl/RstlVersions.h"

#include "types.h"

#include "stddef.h"

#if RSTL_KYOTO_BLOCK_INPUT_STREAM
#if !defined(__MWERKS__) && !defined(CLANGD)
#include <cstdlib>
#include <type_traits>
#endif
#else
#include "Kyoto/Basics/CBasics.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/pair.hpp"
#endif

// Two implementations share this interface:
// - the block stream (Prime and the Echoes demos) reads blocks from a derived stream's
//   virtual Read, and its typed reads go through cinput_stream_helper;
// - the buffer stream (Echoes and Corruption) reads directly from a memory buffer, and its typed
//   reads are specializations of Get.

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

#if !RSTL_KYOTO_BLOCK_INPUT_STREAM
// The buffer stream's reader bodies. Each is shared by a typed name and Prime's name for the same
// read; the two must stay exact copies (a forwarding call changes Echoes' inlining).
#if RSTL_KYOTO_INPUT_STREAM_TYPED_CURSOR
#define CINPUTSTREAM_READ_U32                                                                      \
  uint* result = mpBufferAt.p32++;                                                                 \
  return BE_SWAP(*result)
#define CINPUTSTREAM_READ_U16 return BE_SWAP(*mpBufferAt.p16++)
#define CINPUTSTREAM_READ_I8 return *mpBufferAt.p8++
#define CINPUTSTREAM_READ_U64                                                                      \
  u64* result = reinterpret_cast< u64* >(mpBufferAt.p8);                                           \
  mpBufferAt.p8 = reinterpret_cast< uchar* >(result + 1);                                          \
  return BE_SWAP(*result)
#else
#define CINPUTSTREAM_READ_U32                                                                      \
  uint* result = reinterpret_cast< uint* >(mpBufferAt.p8);                                         \
  mpBufferAt.p8 += sizeof(uint);                                                                   \
  return BE_SWAP(*result)
#define CINPUTSTREAM_READ_U16                                                                      \
  ushort* result = reinterpret_cast< ushort* >(mpBufferAt.p8);                                     \
  mpBufferAt.p8 = reinterpret_cast< uchar* >(result + 1);                                          \
  return BE_SWAP(*result)
#define CINPUTSTREAM_READ_I8 return static_cast< char >(ReadUint8())
#define CINPUTSTREAM_READ_U64                                                                      \
  u64* result = reinterpret_cast< u64* >(mpBufferAt.p8);                                           \
  mpBufferAt.p8 += sizeof(u64);                                                                    \
  return BE_SWAP(*result)
#endif
#endif

class CInputStream {
public:
#if !RSTL_KYOTO_BLOCK_INPUT_STREAM
  struct SBufferAndSize {
    const void* mBuffer;
    unsigned long mSize;

    SBufferAndSize(const void* buffer, unsigned long size) : mBuffer(buffer), mSize(size) {}
  };
#endif

#if RSTL_KYOTO_BLOCK_INPUT_STREAM
  CInputStream(int len);
  CInputStream(const void* ptr, int len, bool owned);
#else
  CInputStream(const void* ptr, unsigned long len);
  CInputStream(const void* ptr, unsigned long len, bool owned);
  CInputStream(const SBufferAndSize& buffer, bool owned);
#endif
  virtual ~CInputStream();
#if RSTL_KYOTO_BLOCK_INPUT_STREAM
  virtual size_t Read(void* dest, size_t len) = 0;
#endif

  float ReadFloat();
  double ReadDouble();
#if RSTL_KYOTO_BLOCK_INPUT_STREAM
  u64 ReadLongLong();
  uint ReadLong();
  ushort ReadShort();
  bool ReadBool();
  char ReadChar();
  uint ReadBits(uint len);
#endif
  size_t ReadBytes(void* dest, size_t len);
  void Get(void* dest, unsigned long len);
#if !RSTL_KYOTO_BLOCK_INPUT_STREAM
  const void* Get(unsigned long len);
  rstl::auto_ptr< uchar > ReleaseBuffer();
#endif

  // Reads a T: the specializations below for scalars, T(CInputStream&) otherwise.
  template < typename T >
  T Get(const TType< T >& type = TType< T >())
#if RSTL_KYOTO_BLOCK_INPUT_STREAM
  {
    return cinput_stream_helper(TType< T >(), *this);
  }
#else
      ;
#endif

  // Guessed name for the typed object read-into operation used by script properties.
  template < typename T >
  void Read(T& value) {
    value = T(*this);
  }

#if RSTL_KYOTO_BLOCK_INPUT_STREAM
  bool ReadPackedBool() { return ReadBits(1) != 0; }
#endif

  // The typed names (Echoes and Corruption). The block stream's go through Get, except the
  // byte reads, which take the next byte of the current block.
  uchar ReadUint8() {
#if RSTL_KYOTO_BLOCK_INPUT_STREAM
    const uchar result = *mPtr++;
#else
    const uchar result = *mpBufferAt.p8++;
#endif
    return result;
  }
#if RSTL_KYOTO_BLOCK_INPUT_STREAM
  char ReadInt8() { return static_cast< char >(ReadUint8()); }
#else
  char ReadInt8() { CINPUTSTREAM_READ_I8; }
#endif
  int ReadInt32() {
#if RSTL_KYOTO_BLOCK_INPUT_STREAM
    return Get< int >();
#else
    int* result = reinterpret_cast< int* >(mpBufferAt.p8);
    mpBufferAt.p8 += sizeof(int);
    return BE_SWAP(*result);
#endif
  }
#if RSTL_KYOTO_BLOCK_INPUT_STREAM
  uint ReadUint32() { return Get< uint >(); } // Guessed name.
  u64 ReadInt64() { return ReadLongLong(); }
  ushort ReadUint16() { return Get< ushort >(); }
  short ReadInt16() { return Get< short >(); }
#else
  uint ReadUint32() { CINPUTSTREAM_READ_U32; } // Guessed name.
  u64 ReadInt64() { CINPUTSTREAM_READ_U64; }
  ushort ReadUint16() { CINPUTSTREAM_READ_U16; }
  short ReadInt16() { return static_cast< short >(ReadUint16()); }

  // Prime's names. The block stream defines these out of line.
  bool ReadBool() {
#if RSTL_KYOTO_INPUT_STREAM_TYPED_CURSOR
    return ReadUint8() != 0;
#else
    return *mpBufferAt.p8++ != 0;
#endif
  }
  uint ReadLong() { CINPUTSTREAM_READ_U32; }
  ushort ReadShort() { CINPUTSTREAM_READ_U16; }
  char ReadChar() { CINPUTSTREAM_READ_I8; }
  u64 ReadLongLong() { CINPUTSTREAM_READ_U64; }
#endif

#if RSTL_KYOTO_BLOCK_INPUT_STREAM
  uint GetBlockOffset() const { return mBlockOffset; }
  const uint GetReadPosition() const { return mReadPosition; }

private:
#if RSTL_ECHOES_DEMO
  void GrabAnotherBlock();
#else
  bool GrabAnotherBlock();
#endif
  bool InternalReadNext();

  uint mBlockOffset;
  uint mBlockLen;
  uint mLen;
  uchar* mPtr;
  bool mOwned;
  uint mReadPosition;
  uint mBitWord;
  uint mBitOffset;
#else
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
#endif
};

#if RSTL_KYOTO_BLOCK_INPUT_STREAM
DECLARE_FULL_SIZE_FOR(CInputStream, 0x24)
#else
DECLARE_FULL_SIZE_FOR(CInputStream, 0x14)
#endif
CHECK_SIZEOF(CInputStream, CInputStream_FULL_SIZE)

#undef CINPUTSTREAM_READ_U32
#undef CINPUTSTREAM_READ_U16
#undef CINPUTSTREAM_READ_I8
#undef CINPUTSTREAM_READ_U64

// The typed reads. CINPUTSTREAM_GET(T, block, buffer) names the block stream's reader, then the
// buffer stream's.
#if RSTL_KYOTO_BLOCK_INPUT_STREAM
template < typename T >
inline T cinput_stream_helper(const TType< T >& type, CInputStream& in) {
#if !defined(__MWERKS__) && !defined(CLANGD)
  if constexpr (!std::is_constructible_v< T, CInputStream& >) {
    abort();
  } else
#endif
  {
    return T(in);
  }
}
#define CINPUTSTREAM_GET(T, blockRead, bufferRead)                                                 \
  template <>                                                                                      \
  inline T cinput_stream_helper(const TType< T >& type, CInputStream& in) {                        \
    return in.blockRead;                                                                           \
  }
#else
template < typename T >
inline T CInputStream::Get(const TType< T >& type) {
  return T(*this);
}
#define CINPUTSTREAM_GET(T, blockRead, bufferRead)                                                 \
  template <>                                                                                      \
  inline T CInputStream::Get< T >(const TType< T >& type) {                                        \
    return bufferRead;                                                                             \
  }
#endif

// The typed cursor reads chars through its char member, and unsigned longs as unsigned.
#if RSTL_KYOTO_INPUT_STREAM_TYPED_CURSOR
#define CINPUTSTREAM_GET_CHAR *mpBufferAt.pc8++
#define CINPUTSTREAM_GET_ULONG ReadUint32()
#else
#define CINPUTSTREAM_GET_CHAR ReadInt8()
#define CINPUTSTREAM_GET_ULONG ReadInt32()
#endif

CINPUTSTREAM_GET(bool, ReadBool(), ReadBool())
CINPUTSTREAM_GET(char, ReadChar(), CINPUTSTREAM_GET_CHAR)
CINPUTSTREAM_GET(unsigned char, ReadChar(), ReadUint8())
#if RSTL_KYOTO_INPUT_STREAM_TYPED_CURSOR
// Read as a char through a local tag: a temporary tag would emit a 1-byte .sbss object.
template <>
inline signed char CInputStream::Get< signed char >(const TType< signed char >& type) {
  TType< char > charType;
  return Get(charType);
}
#else
CINPUTSTREAM_GET(signed char, ReadChar(), ReadInt8())
#endif
CINPUTSTREAM_GET(int, ReadLong(), ReadInt32())
CINPUTSTREAM_GET(uint, ReadLong(), ReadUint32())
CINPUTSTREAM_GET(unsigned long, ReadLong(), CINPUTSTREAM_GET_ULONG)
CINPUTSTREAM_GET(u64, ReadLongLong(), ReadInt64())
CINPUTSTREAM_GET(float, ReadFloat(), ReadFloat())
CINPUTSTREAM_GET(short, ReadShort(), ReadInt16())
CINPUTSTREAM_GET(ushort, ReadShort(), ReadUint16())

#undef CINPUTSTREAM_GET
#undef CINPUTSTREAM_GET_CHAR
#undef CINPUTSTREAM_GET_ULONG

// Containers declared early, in the order Retro's headers included them.
#if RSTL_KYOTO_BLOCK_INPUT_STREAM
#include "rstl/pair.hpp"

#include "rstl/vector.hpp"

#include "rstl/reserved_vector.hpp"

#include "rstl/set.hpp"
#else
#include "rstl/vector.hpp"

#include "rstl/reserved_vector.hpp"

#include "rstl/red_black_tree.hpp"
#endif

namespace rstl {
template < typename L, typename R >
inline pair< L, R >::pair(CInputStream& in)
: first(in.Get(TGetType(first))), second(in.Get(TGetType(second))) {}

template < typename T, int N >
inline reserved_vector< T, N >::reserved_vector(CInputStream& in)
#if RSTL_STREAM_READ_INT32
: mCount(in.ReadInt32()) {
#else
: mCount(in.Get(TGetType(0))) {
#endif
  for (int i = 0; i < mCount; i++) {
    const TType< T >& type = TType< T >();
    if (reserved_vector_push_policy< T >::value) {
      reserved_vector_push_policy< T >::construct_element(&data()[i], in.Get(type));
    } else {
      construct(&data()[i], in.Get(type));
    }
  }
}

} // namespace rstl

#endif // _CINPUTSTREAM
