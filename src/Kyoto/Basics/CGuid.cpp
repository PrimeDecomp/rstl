#include "Kyoto/Basics/CGuid.hpp"

#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"

#if RSTL_VERSION == RSTL_G2MEAB
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Basics/CStopwatch.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Streams/COutputStreamVector.hpp"

#include <ctype.h>
#include <string.h>
#endif

CGuid::CGuid(CInputStream& in) { in.Get(mData, sizeof(mData)); }

void CGuid::PutTo(COutputStream& out) const { out.DoPut(mData, sizeof(mData)); }

#if RSTL_VERSION == RSTL_G2MEAB
CGuid CGuid::sDefault(rstl::string_l("{00000000-0000-0000-0000-000000000000}"));

CGuid::CGuid(const rstl::string& text) { ParseString(text); }

bool CGuid::operator==(const CGuid& other) const {
  const u64* lhs = reinterpret_cast< const u64* >(mData);
  const u64* rhs = reinterpret_cast< const u64* >(other.mData);
  return lhs[0] == rhs[0] && lhs[1] == rhs[1];
}

// These vector-stream methods belong to the native CGuid translation unit.
COutputStreamVector::COutputStreamVector(int bufferLength) : COutputStream(bufferLength) {}

COutputStreamVector::~COutputStreamVector() { DoFlush(); }

const rstl::vector< uchar >& COutputStreamVector::GetBuffer() {
  DoFlush();
  return mBytes;
}

void COutputStreamVector::Write(const void* data, size_t length) {
  const uchar* bytes = static_cast< const uchar* >(data);
  mBytes.insert(mBytes.end(), bytes, bytes + length);
}

CGuid::CGuid(const SGuidData& data) : mFields(data) {}

CGuid CGuid::Generate() {
  static int counter = 0;
  CRandom16 random(CStopwatch::GetGlobalMicros() + counter);
  ++counter;
  SGuidData data;
  const uint high = uint(random.Next()) << 16;
  data.mData1 = random.Next() + high;
  data.mData2 = random.Next();
  data.mData3 = random.Next();
  data.mData4[0] = random.Next();
  data.mData4[1] = random.Next();
  data.mData4[2] = random.Next();
  data.mData4[3] = random.Next();
  data.mData4[4] = random.Next();
  data.mData4[5] = random.Next();
  data.mData4[6] = random.Next();
  data.mData4[7] = random.Next();
  return CGuid(data);
}

rstl::string CGuid::ToString() const {
  // Guessed name. Native rodata keeps this format separately from the default GUID text.
  static const char kFormat[] = "{%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x}";
  return rstl::string(CBasics::Stringize(
      kFormat,
      mBytes[0], mBytes[1], mBytes[2], mBytes[3], mBytes[4], mBytes[5], mBytes[6], mBytes[7],
      mBytes[8], mBytes[9], mBytes[10], mBytes[11], mBytes[12], mBytes[13], mBytes[14], mBytes[15]));
}

void CGuid::ParseString(const rstl::string& text) {
  // The prototype checks 36 characters, stores individual digits in a 16-byte buffer,
  // and resets the result even after copying it. This unfinished behavior is native.
  if (text.length() == 36) {
    uchar data[16];
    uchar* dst = data;
    for (int i = 1; i < 35; ++i) {
      switch (i) {
      case 9:
      case 14:
      case 19:
      case 24:
        break;
      default: {
        const int c = toupper(text[i]);
        if (isxdigit(c)) {
          int digit = c - '0';
          if (digit > 9) {
            digit -= 'A' - '0';
          }
          *dst++ = digit;
        } else {
          *this = sDefault;
          return;
        }
        break;
      }
      }
    }
    memcpy(mData, data, sizeof(mData));
  }
  *this = sDefault;
}
#endif
