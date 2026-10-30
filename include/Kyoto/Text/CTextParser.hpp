#ifndef _CTEXTPARSER
#define _CTEXTPARSER
#include "rstl/RstlVersions.h"

#include "Kyoto/TToken.hpp"
#include "Kyoto/Text/CFontImageDef.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "rstl/pair.hpp"
#include "rstl/vector.hpp"

#include "rstl/string.hpp"
#include "types.h"

#include "Kyoto/Text/CTextColor.hpp"

class IObjectStore;
#if !RSTL_PRIME1
class CTextExecuteBuffer;
#endif

class CRasterFont;
#if RSTL_PRIME1
class CTextExecuteBuffer;
#endif

class CTextParser {
public:
  CTextParser(IObjectStore& store);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void ParseText(CTextExecuteBuffer&, const wchar_t* str, int len,
                 const rstl::vector< rstl::pair< CAssetId, CAssetId > >* vec);
  static const CAssetId
#else
  void ParseText(CTextExecuteBuffer& buffer, const wchar_t* str, int len,
                 const rstl::vector< rstl::pair< CAssetId, CAssetId > >* textureMap);
  static CAssetId
#endif

  GetAssetIdFromString(const rstl::string& str,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                       const rstl::vector< rstl::pair< CAssetId, CAssetId > >* txtrMap);
#else
                       const rstl::vector< rstl::pair< CAssetId, CAssetId > >* textureMap);
#endif

  TToken< CRasterFont > GetFont(const wchar_t* str, int len);
  CFontImageDef GetImage(const wchar_t* str, int len,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                         const rstl::vector< rstl::pair< CAssetId, CAssetId > >* vec);
#else
                         const rstl::vector< rstl::pair< CAssetId, CAssetId > >* textureMap);
#endif

  uint HandleUserTag(CTextExecuteBuffer& buffer, const wchar_t* str, int len);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void ParseTag(CTextExecuteBuffer&, const wchar_t* str, int len,
                const rstl::vector< rstl::pair< CAssetId, CAssetId > >* vec);
#else
  void ParseTag(CTextExecuteBuffer& buffer, const wchar_t* str, int len,
                const rstl::vector< rstl::pair< CAssetId, CAssetId > >* textureMap);
#endif

  static bool BeginsWith(const wchar_t* str1, int len, const wchar_t* str2);
  static bool Equals(const wchar_t* str1, int len, const wchar_t* str2);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static int ParseInt(const wchar_t* str, int len, bool);
#else
  static int ParseInt(const wchar_t* str, int len, bool allowSign);
#endif

  static int FromHex(wchar_t c);
// TODO: verify that this Prime/Echoes difference is real
  static RSTL_PRIME1_CONST int GetColorValue(const wchar_t* str);

  CTextColor ParseColor(const wchar_t* str, int len);

private:
  IObjectStore& mObjectStore;
};

CHECK_SIZEOF(CTextParser, 0x4)

#endif // _CTEXTPARSER
