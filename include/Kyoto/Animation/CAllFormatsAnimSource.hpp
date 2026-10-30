#ifndef _CALLFORMATSANIMSOURCE
#define _CALLFORMATSANIMSOURCE
#include "rstl/RstlVersions.h"
#include "types.h"

#include "Kyoto/Animation/CAnimSource.hpp"
#include "Kyoto/Animation/CFBStreamedCompression.hpp"
#include "Kyoto/Animation/IAnimReader.hpp"
#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/TToken.hpp"
#include "Kyoto/SObjectTag.hpp"
#if !RSTL_PRIME1
#include "Kyoto/Animation/TSubAnimTypeToken.hpp"
#endif

#include "Kyoto/CFactoryFnReturn.hpp"

class CInputStream;

class IObjectStore;
class CAnimPOIData;

class CAnimFormatUnion {
public:
#if !RSTL_PRIME1
  // Format names reconstructed from the concrete classes selected by the target.
  enum EFormat { kF_AnimSource = 0, kF_FBStreamedCompression = 2 };
#endif

  CAnimFormatUnion(CInputStream& in, IObjectStore& store);
  ~CAnimFormatUnion();

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int GetType() const { return mFormatType; }
  CAnimSource& AsCAnimSource() { return *reinterpret_cast< CAnimSource* >(mFormatData); }
#else
  uint GetType() const { return mFormatType; }
#endif

  const CAnimSource& AsCAnimSource() const {
    return *reinterpret_cast< const CAnimSource* >(mFormatData);
  }
#if RSTL_PRIME1
  CFBStreamedCompression& AsCFBStreamedCompression() {
    return *reinterpret_cast< CFBStreamedCompression* >(mFormatData);
  }
#endif
  const CFBStreamedCompression& AsCFBStreamedCompression() const {
    return *reinterpret_cast< const CFBStreamedCompression* >(mFormatData);
  }

  static void SubConstruct(uchar* ptr, const uint format, CInputStream& in, IObjectStore& store);

private:
  uint mFormatType;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  ALIGNAS(CAnimSource) ALIGNAS(CFBStreamedCompression)
  uchar mFormatData[sizeof(CAnimSource) > sizeof(CFBStreamedCompression)
                        ? sizeof(CAnimSource) : sizeof(CFBStreamedCompression)];
#else
  ATTRIBUTE_ALIGN_DECL(4, uchar mFormatData[sizeof(CAnimSource) > sizeof(CFBStreamedCompression)
                                                ? sizeof(CAnimSource)
                                                : sizeof(CFBStreamedCompression)]);
#endif
};
#if !RSTL_PRIME1
CHECK_SIZEOF(CAnimFormatUnion, 0x88)
#endif

class CAllFormatsAnimSource {
public:
  CAllFormatsAnimSource(CInputStream& in, IObjectStore& store, const SObjectTag& tag);
  ~CAllFormatsAnimSource();

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int GetType() const { return mFormatUnion.GetType(); }
#else
  uint GetType() const { return mFormatUnion.GetType(); }
#endif

  CCharAnimTime GetAnimationDuration() const {
    switch (GetType()) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    case 0:
#else
    case CAnimFormatUnion::kF_AnimSource:
#endif
      return AsCAnimSource().GetAnimationDuration();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    case 2:
#else
    case CAnimFormatUnion::kF_FBStreamedCompression:
#endif
      return AsCFBStreamedCompression().GetAnimationDuration();
    default:
      return AsCAnimSource().GetAnimationDuration();
    }
  }
  float GetAverageVelocity() const {
    switch (GetType()) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    case 0:
#else
    case CAnimFormatUnion::kF_AnimSource:
#endif
      return AsCAnimSource().GetAverageVelocity();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    case 2:
#else
    case CAnimFormatUnion::kF_FBStreamedCompression:
#endif
      return AsCFBStreamedCompression().GetAverageVelocity();
    default:
      return AsCAnimSource().GetAverageVelocity();
    }
  }
  const CAnimSource& AsCAnimSource() const { return mFormatUnion.AsCAnimSource(); }
  const CFBStreamedCompression& AsCFBStreamedCompression() const {
    return mFormatUnion.AsCFBStreamedCompression();
  }
  void GetFormatPointer(const CAnimSource*& ptr) const { ptr = &AsCAnimSource(); }
  void GetFormatPointer(const CFBStreamedCompression*& ptr) const {
    ptr = &AsCFBStreamedCompression();
  }

  static rstl::ownership_transfer< IAnimReader >
  GetNewReader(const TLockedToken< CAllFormatsAnimSource >& tok, const CCharAnimTime& time
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
               , const CAnimPOIData* poiData
#endif
               );

private:
  CAnimFormatUnion mFormatUnion;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CVector3f x68_;
#else
  CVector3f x88_;
#endif

  SObjectTag mTag;
};
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CHECK_SIZEOF(CAnimFormatUnion, 0x68)
CHECK_SIZEOF(CAllFormatsAnimSource, 0x7c)
#else
CHECK_SIZEOF(CAllFormatsAnimSource, 0x9c)
#endif

template < typename T >
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
class TSubAnimTypeToken {
public:
  TSubAnimTypeToken(const TLockedToken< CAllFormatsAnimSource >& tok)
#else
inline TSubAnimTypeToken< T >::TSubAnimTypeToken(const TLockedToken< CAllFormatsAnimSource >& tok)
#endif
  : mToken(tok) {
    mToken->GetFormatPointer(mSource);
  }
#if RSTL_PRIME1
  const T* const& GetSource() const { return mSource; }
  const T* operator->() const { return mSource; }
  const T& operator*() const { return *mSource; }

private:
  TLockedToken< CAllFormatsAnimSource > mToken;
  const T* mSource;
};
#endif

RSTL_PRIME1_CONST CFactoryFnReturn AnimSourceFactory(const SObjectTag& tag, CInputStream& in,
                                                     const CVParamTransfer& param);

#endif // _CALLFORMATSANIMSOURCE
