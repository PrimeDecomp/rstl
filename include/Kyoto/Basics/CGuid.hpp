#ifndef _CGUID
#define _CGUID

#include "types.h"

#include "rstl/string.hpp"
#include "rstl/RstlVersions.h"

class CInputStream;
class COutputStream;

// A 128-bit globally unique identifier; newer games confirm the class and its name. The BBA
// support pairs a request to the host with its reply by it. If a class that inherits CGuid is
// found, it is CObjectId.
class CGuid {
public:
#if RSTL_CORRUPTION
  // Guessed names. The fields used when generating the prototype's sixteen-byte identifier.
  struct SGuidData {
    uint mData1;
    ushort mData2;
    ushort mData3;
    uchar mData4[8];
  };

  explicit CGuid(const rstl::string& text);
  explicit CGuid(const SGuidData& data);
  // Guessed name. Preserves the prototype's incomplete text parser.
  void ParseString(const rstl::string& text);
#endif
  // Guessed name. Reads the sixteen bytes.
  explicit CGuid(CInputStream& in);
  // Guessed name. Writes the sixteen bytes.
  void PutTo(COutputStream& out) const;
  static CGuid Generate();

  // Guessed name. The text form, "{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}".
  rstl::string ToString() const;
  bool operator==(const CGuid& other) const;

  // Guessed name. The all zero guid, parsed at startup.
  static CGuid sDefault;

#if RSTL_CORRUPTION
  // Guessed names. Byte formatting and structured generation views; the native record is four-byte aligned.
  union {
    uint mData[4];
    uchar mBytes[16];
    SGuidData mFields;
  };
#else
  uint mData[4];
#endif
};

#endif // _CGUID
