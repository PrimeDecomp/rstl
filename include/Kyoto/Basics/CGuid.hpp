#ifndef _CGUID
#define _CGUID

#include "types.h"

#include "rstl/string.hpp"

class CInputStream;
class COutputStream;

// A 128-bit globally unique identifier; newer games confirm the class and its name. The BBA
// support pairs a request to the host with its reply by it. If a class that inherits CGuid is
// found, it is CObjectId.
class CGuid {
public:
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

  uint mData[4];
};

#endif // _CGUID
