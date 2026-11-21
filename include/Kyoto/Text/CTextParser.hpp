#ifndef _CTEXTPARSER
#define _CTEXTPARSER
#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"
#endif

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#include "Kyoto/SObjectTag.hpp"
#endif
#include "Kyoto/TToken.hpp"
#include "Kyoto/Text/CFontImageDef.hpp"
#if RSTL_CORRUPTION
#include "rstl/optional_object.hpp"
#else
#include "Kyoto/SObjectTag.hpp"
#endif
#include "rstl/pair.hpp"
#include "rstl/vector.hpp"

#include "rstl/string.hpp"
#include "rstl/types.h"

#include "Kyoto/Text/CTextColor.hpp"

class IObjectStore;
class CTextExecuteBuffer;

class CRasterFont;
#if RSTL_CORRUPTION

CAssetId ParseWideHexAssetIdentifier(const wchar_t* str); // Guessed name
#endif

class CTextParser {
public:
  CTextParser(IObjectStore& store);
  void ParseText(CTextExecuteBuffer& buffer, const wchar_t* str, int len,
                 const rstl::vector< rstl::pair< CAssetId, CAssetId > >* textureMap);
#if RSTL_CORRUPTION
  static CAssetId
#else
  static RSTL_PRIME1_CONST CAssetId
#endif
  GetAssetIdFromString(const rstl::string& str,
                       const rstl::vector< rstl::pair< CAssetId, CAssetId > >* textureMap);
#if RSTL_CORRUPTION
  rstl::optional_object< TToken< CRasterFont > > GetFont(const wchar_t* str, int len);
  rstl::optional_object< CFontImageDef >
  GetImage(const wchar_t* str, int len,
#else

  TToken< CRasterFont > GetFont(const wchar_t* str, int len);
  CFontImageDef GetImage(const wchar_t* str, int len,
#endif
                         const rstl::vector< rstl::pair< CAssetId, CAssetId > >* textureMap);

  uint HandleUserTag(CTextExecuteBuffer& buffer, const wchar_t* str, int len);
  void ParseTag(CTextExecuteBuffer& buffer, const wchar_t* str, int len,
                const rstl::vector< rstl::pair< CAssetId, CAssetId > >* textureMap);

  static bool BeginsWith(const wchar_t* str1, int len, const wchar_t* str2);
  static bool Equals(const wchar_t* str1, int len, const wchar_t* str2);
  static int ParseInt(const wchar_t* str, int len, bool allowSign);
#if RSTL_CORRUPTION
  static int FromHex(const wchar_t c);
  static int GetColorValue(const wchar_t* str);
#else

  static int FromHex(wchar_t c);
  static RSTL_PRIME1_CONST int GetColorValue(const wchar_t* str);

#endif
  CTextColor ParseColor(const wchar_t* str, int len);

private:
  IObjectStore& mObjectStore;
};

CHECK_SIZEOF(CTextParser, 0x4)

#endif // _CTEXTPARSER
