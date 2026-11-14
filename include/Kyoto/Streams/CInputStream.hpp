#ifndef _CINPUTSTREAM
#define _CINPUTSTREAM

#include "rstl/RstlVersions.h"
#include "rstl/static_assert.hpp"

#include "rstl/types.h"

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

// Two implementations share this interface (RSTL_KYOTO_BLOCK_INPUT_STREAM selects one):
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
  size_t ReadBytes(void* dest, size_t len);
  void Get(void* dest, unsigned long len);
#if !RSTL_KYOTO_BLOCK_INPUT_STREAM
  const void* Get(unsigned long len);
  rstl::auto_ptr< uchar > ReleaseBuffer();
#endif

  // Reads a T: the specializations below for scalars, T(CInputStream&) otherwise.
  // The block stream defines Get and its four wrappers below in the class: GC/1.3.2 emits Prime's
  // 1-byte .sbss TType objects in every user only in this form.
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
  void Read(T& value);

  // Prime's names, from the demo map. The block stream defines these out of line.
  bool ReadBool();
  char ReadChar();
  ushort ReadShort();
  uint ReadLong();
  u64 ReadLongLong();
#if RSTL_KYOTO_BLOCK_INPUT_STREAM
  uint ReadBits(uint len);
  bool ReadPackedBool();
#endif

  // The typed names (Echoes and Corruption).
  uchar ReadUint8();
  char ReadInt8();
  u64 ReadInt64();
#if RSTL_KYOTO_BLOCK_INPUT_STREAM
  int ReadInt32() { return Get< int >(); }
  uint ReadUint32() { return Get< uint >(); } // Guessed name.
  ushort ReadUint16() { return Get< ushort >(); }
  short ReadInt16() { return Get< short >(); }
#else
  int ReadInt32();
  uint ReadUint32(); // Guessed name.
  ushort ReadUint16();
  short ReadInt16();
#endif

#if RSTL_KYOTO_BLOCK_INPUT_STREAM
  uint GetBlockOffset() const;
  const uint GetReadPosition() const;

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
  uint GetReadPosition() const;

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

template < typename T >
inline void CInputStream::Read(T& value) {
  value = T(*this);
}

inline uchar CInputStream::ReadUint8() {
#if RSTL_KYOTO_BLOCK_INPUT_STREAM
  const uchar result = *mPtr++;
#else
  const uchar result = *mpBufferAt.p8++;
#endif
  return result;
}

#if RSTL_KYOTO_BLOCK_INPUT_STREAM
// The block stream. Its scalar reads go through cinput_stream_helper to the out-of-line readers,
// except the byte reads, which take the next byte of the current block.

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

#define CINPUTSTREAM_HELPER(T, read)                                                               \
  template <>                                                                                      \
  inline T cinput_stream_helper(const TType< T >& type, CInputStream& in) {                        \
    return in.read();                                                                              \
  }
CINPUTSTREAM_HELPER(bool, ReadBool)
CINPUTSTREAM_HELPER(char, ReadChar)
CINPUTSTREAM_HELPER(unsigned char, ReadChar)
CINPUTSTREAM_HELPER(signed char, ReadChar)
CINPUTSTREAM_HELPER(int, ReadLong)
CINPUTSTREAM_HELPER(uint, ReadLong)
CINPUTSTREAM_HELPER(unsigned long, ReadLong)
CINPUTSTREAM_HELPER(u64, ReadLongLong)
CINPUTSTREAM_HELPER(float, ReadFloat)
CINPUTSTREAM_HELPER(short, ReadShort)
CINPUTSTREAM_HELPER(ushort, ReadShort)
#undef CINPUTSTREAM_HELPER

inline bool CInputStream::ReadPackedBool() { return ReadBits(1) != 0; }
inline char CInputStream::ReadInt8() { return static_cast< char >(ReadUint8()); }
inline u64 CInputStream::ReadInt64() { return ReadLongLong(); }
inline uint CInputStream::GetBlockOffset() const { return mBlockOffset; }
inline const uint CInputStream::GetReadPosition() const { return mReadPosition; }

#include "rstl/pair.hpp"

#include "rstl/vector.hpp"

#include "rstl/reserved_vector.hpp"

#include "rstl/set.hpp"

#else
// The buffer stream. Each read is defined twice, under its typed name and Prime's; the copies must
// stay exact (a forwarding call changes Echoes' inlining), so their bodies are shared macros.
// The typed cursor (RSTL_KYOTO_INPUT_STREAM_TYPED_CURSOR) steps through the union's typed members.

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

inline char CInputStream::ReadInt8() { CINPUTSTREAM_READ_I8; }
inline char CInputStream::ReadChar() { CINPUTSTREAM_READ_I8; }
inline int CInputStream::ReadInt32() {
  int* result = reinterpret_cast< int* >(mpBufferAt.p8);
  mpBufferAt.p8 += sizeof(int);
  return BE_SWAP(*result);
}
inline uint CInputStream::ReadUint32() { CINPUTSTREAM_READ_U32; }
inline uint CInputStream::ReadLong() { CINPUTSTREAM_READ_U32; }
inline u64 CInputStream::ReadInt64() { CINPUTSTREAM_READ_U64; }
inline u64 CInputStream::ReadLongLong() { CINPUTSTREAM_READ_U64; }
inline ushort CInputStream::ReadUint16() { CINPUTSTREAM_READ_U16; }
inline ushort CInputStream::ReadShort() { CINPUTSTREAM_READ_U16; }
inline short CInputStream::ReadInt16() { return static_cast< short >(ReadUint16()); }
inline bool CInputStream::ReadBool() {
#if RSTL_KYOTO_INPUT_STREAM_TYPED_CURSOR
  return ReadUint8() != 0;
#else
  return *mpBufferAt.p8++ != 0;
#endif
}
inline uint CInputStream::GetReadPosition() const { return mpBufferAt.p8 - mpBufferStart; }

#undef CINPUTSTREAM_READ_U32
#undef CINPUTSTREAM_READ_U16
#undef CINPUTSTREAM_READ_I8
#undef CINPUTSTREAM_READ_U64

template < typename T >
inline T CInputStream::Get(const TType< T >& type) {
  return T(*this);
}

#define CINPUTSTREAM_GET(T, read)                                                                  \
  template <>                                                                                      \
  inline T CInputStream::Get< T >(const TType< T >& type) {                                        \
    return read;                                                                                   \
  }
CINPUTSTREAM_GET(bool, ReadBool())
#if RSTL_KYOTO_INPUT_STREAM_TYPED_CURSOR
CINPUTSTREAM_GET(char, *mpBufferAt.pc8++)
#else
CINPUTSTREAM_GET(char, ReadInt8())
#endif
CINPUTSTREAM_GET(unsigned char, ReadUint8())
#if RSTL_KYOTO_INPUT_STREAM_TYPED_CURSOR
// Read as a char through a local tag: a temporary tag would emit a 1-byte .sbss object.
template <>
inline signed char CInputStream::Get< signed char >(const TType< signed char >& type) {
  TType< char > charType;
  return Get(charType);
}
#else
CINPUTSTREAM_GET(signed char, ReadInt8())
#endif
CINPUTSTREAM_GET(int, ReadInt32())
CINPUTSTREAM_GET(uint, ReadUint32())
#if RSTL_KYOTO_INPUT_STREAM_TYPED_CURSOR
CINPUTSTREAM_GET(unsigned long, ReadUint32())
#else
CINPUTSTREAM_GET(unsigned long, ReadInt32())
#endif
CINPUTSTREAM_GET(u64, ReadInt64())
CINPUTSTREAM_GET(float, ReadFloat())
CINPUTSTREAM_GET(short, ReadInt16())
CINPUTSTREAM_GET(ushort, ReadUint16())
#undef CINPUTSTREAM_GET

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
