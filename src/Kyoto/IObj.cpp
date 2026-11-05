#include "rstl/RstlVersions.h"
#include "Kyoto/IObj.hpp"

#if RSTL_KYOTO_ASSET_ID_CLASS
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"
#endif

#include <ctype.h>
#if RSTL_KYOTO_ASSET_ID_CLASS
#include <stdio.h>
#endif

#if RSTL_KYOTO_ASSET_ID_CLASS
const CAssetId kInvalidAssetId(-1);
#endif
const SObjectTag gkInvalidObjectTag(-1, kInvalidAssetId);

const char* SObjectTag::Type2Text(FourCC type) {
  static char text[5];
  text[0] = toupper((type >> 24) & 0xFF);
  text[1] = toupper((type >> 16) & 0xFF);
  text[2] = toupper((type >> 8) & 0xFF);
  text[3] = toupper(type & 0xFF);
  text[4] = '\0';

  for (int i = 0; i < 4; ++i) {
    if (isprint(text[i]) == 0) {
      text[i] = '-';
    }
  }
  return text;
}

#if RSTL_KYOTO_ASSET_ID_CLASS
FourCC SObjectTag::Text2Type(const char* text) {
  FourCC type = 0;
  if (text != NULL && isprint(text[0]) && isprint(text[1]) && isprint(text[2]) &&
      isprint(text[3])) {
    type = (toupper(text[0]) << 24) | (toupper(text[1]) << 16) | (toupper(text[2]) << 8) |
           toupper(text[3]);
  }
  return type;
}

long long CAssetId::ParseDecimalString(const char* str) {
  long long value = 0;
  int sign = 1;
  while (*str == ' ' || *str == '\t') {
    ++str;
  }
  if (*str == '-') {
    sign = -1;
  }
  while (*str != '\0') {
    if (*str >= '0' && *str <= '9') {
      value = value * 10 + *str++ - '0';
    } else {
      ++str;
    }
  }
  return sign * value;
}

CAssetId::CAssetId(const char* str) {
  if (str[0] != '\0' && str[1] != '\0' && str[0] == '0' && str[1] == 'x') {
    unsigned long long id = 0;
    for (const char* c = str + 2; *c != '\0'; ++c) {
      if (*c >= '0' && *c <= '9') {
        id = (id << 4) | (*c - '0');
      } else if (*c >= 'A' && *c <= 'F') {
        id = (id << 4) | (*c - 'A' + 10);
      } else if (*c >= 'a' && *c <= 'f') {
        id = (id << 4) | (*c - 'a' + 10);
      } else {
        break;
      }
    }
    mId = id;
  } else {
    mId = ParseDecimalString(str);
  }
}

CAssetId::CAssetId(CInputStream& in) : mId(in.ReadInt64()) {}

void CAssetId::PutTo(COutputStream& out) const {
  unsigned long long id = mId;
  out.Put(&id, sizeof(id));
}

rstl::string CAssetId::ToHexString() const {
  char buf[20];
  sprintf(buf, "0x%8.8X%8.8X", static_cast< uint >(mId >> 32), static_cast< uint >(mId));
  return rstl::string(buf);
}
#endif
