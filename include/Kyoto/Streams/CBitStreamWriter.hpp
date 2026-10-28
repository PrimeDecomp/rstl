#ifndef _CBITSTREAMWRITER
#define _CBITSTREAMWRITER

#include "rstl/RstlVersions.h"

#include "types.h"

class COutputStream;

class CBitStreamWriter {
  COutputStream& mStream;
  uint mShiftRegister;
#if RSTL_KYOTO_VOLATILE_SHIFT_OFFSET
  volatile uint mShiftRegisterOffset;
#else
  uint mShiftRegisterOffset;
#endif

public:
  explicit CBitStreamWriter(COutputStream& stream);
  ~CBitStreamWriter();

  uint GetWrittenBits() const;
  void Flush();
  void FlushAll();
  void WriteBits(uint value, uint bitCount);
  void Write64BitsLowWordFirst(u64 value); // Guessed name
  COutputStream& GetOutputStream();
};

CHECK_SIZEOF(CBitStreamWriter, 0xc)

#endif // _CBITSTREAMWRITER
