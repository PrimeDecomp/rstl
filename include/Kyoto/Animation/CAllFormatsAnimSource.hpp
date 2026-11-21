#ifndef _CALLFORMATSANIMSOURCE
#define _CALLFORMATSANIMSOURCE
#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"
#include "rstl/types.h"
#include "rstl/macros.hpp"
#endif

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#endif
#include "Kyoto/Animation/CAnimSource.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Animation/CCompressedAnimSource.hpp"
#endif
#include "Kyoto/Animation/CFBStreamedCompression.hpp"
#include "Kyoto/Animation/IAnimReader.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Animation/TSubAnimTypeToken.hpp"
#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/TToken.hpp"
#else
#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/TToken.hpp"
#include "Kyoto/SObjectTag.hpp"
#if !RSTL_PRIME1
#include "Kyoto/Animation/TSubAnimTypeToken.hpp"
#endif
#endif

#if !RSTL_CORRUPTION
#include "Kyoto/CFactoryFnReturn.hpp"
#endif

class CInputStream;

class IObjectStore;
class CAnimPOIData;

class CAnimFormatUnion {
public:
#if RSTL_CORRUPTION
  enum EFormat {
    kF_AnimSource = 0,
    kF_CompressedAnimSource = 1, // Guessed name
    kF_FBStreamedCompression = 2
  };
#else
#if !RSTL_PRIME1
  // Format names reconstructed from the concrete classes selected by the target.
  enum EFormat { kF_AnimSource = 0, kF_FBStreamedCompression = 2 };
#endif
#endif

  CAnimFormatUnion(CInputStream& in, IObjectStore& store);
  ~CAnimFormatUnion();

#if RSTL_CORRUPTION
  int GetType() const { return mFormatType; }
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int GetType() const { return mFormatType; }
  CAnimSource& AsCAnimSource() { return *reinterpret_cast< CAnimSource* >(mFormatData); }
#else
  uint GetType() const { return mFormatType; }
#endif

#endif
  const CAnimSource& AsCAnimSource() const {
    return *reinterpret_cast< const CAnimSource* >(mFormatData);
  }
#if RSTL_CORRUPTION
  const CCompressedAnimSource& AsCCompressedAnimSource() const { // Guessed name
    return *reinterpret_cast< const CCompressedAnimSource* >(mFormatData);
  }
#else
#if RSTL_PRIME1
  CFBStreamedCompression& AsCFBStreamedCompression() {
    return *reinterpret_cast< CFBStreamedCompression* >(mFormatData);
  }
#endif
#endif
  const CFBStreamedCompression& AsCFBStreamedCompression() const {
    return *reinterpret_cast< const CFBStreamedCompression* >(mFormatData);
  }

  static void SubConstruct(uchar* ptr, const uint format, CInputStream& in, IObjectStore& store);

private:
  uint mFormatType;
#if RSTL_CORRUPTION
  uchar mFormatData[sizeof(CAnimSource) > sizeof(CFBStreamedCompression)
                                                ? sizeof(CAnimSource)
                        : sizeof(CFBStreamedCompression)] ATTRIBUTE_ALIGN(4);
#else
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
#endif
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CAnimFormatUnion, 0x9c)
#else
CHECK_SIZEOF(CAnimFormatUnion, RSTL_PRIME1 ? 0x68 : 0x88)
#endif

class CAllFormatsAnimSource {
public:
  CAllFormatsAnimSource(CInputStream& in, IObjectStore& store, const SObjectTag& tag);
  ~CAllFormatsAnimSource();

#if RSTL_CORRUPTION
  int GetType() const { return mFormatUnion.GetType(); }
  CVector3f GetRootOffset() const { // Guessed name
    switch (GetType()) {
    case CAnimFormatUnion::kF_AnimSource:
      return AsCAnimSource().GetRootOffset();
    case CAnimFormatUnion::kF_CompressedAnimSource:
      return AsCCompressedAnimSource().GetRootOffset();
    case CAnimFormatUnion::kF_FBStreamedCompression:
      return CVector3f::Zero();
    default:
      return AsCAnimSource().GetRootOffset();
    }
  }
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int GetType() const { return mFormatUnion.GetType(); }
#else
  uint GetType() const { return mFormatUnion.GetType(); }
#endif

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
#if RSTL_CORRUPTION
    case CAnimFormatUnion::kF_CompressedAnimSource:
      return AsCCompressedAnimSource().GetAnimationDuration();
    case CAnimFormatUnion::kF_FBStreamedCompression:
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    case 2:
#else
    case CAnimFormatUnion::kF_FBStreamedCompression:
#endif
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
#if RSTL_CORRUPTION
    case CAnimFormatUnion::kF_CompressedAnimSource:
      return AsCCompressedAnimSource().GetAverageVelocity();
    case CAnimFormatUnion::kF_FBStreamedCompression:
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    case 2:
#else
    case CAnimFormatUnion::kF_FBStreamedCompression:
#endif
#endif
      return AsCFBStreamedCompression().GetAverageVelocity();
    default:
      return AsCAnimSource().GetAverageVelocity();
    }
  }
  const CAnimSource& AsCAnimSource() const { return mFormatUnion.AsCAnimSource(); }
#if RSTL_CORRUPTION
  const CCompressedAnimSource& AsCCompressedAnimSource() const { // Guessed name
    return mFormatUnion.AsCCompressedAnimSource();
  }
#endif
  const CFBStreamedCompression& AsCFBStreamedCompression() const {
    return mFormatUnion.AsCFBStreamedCompression();
  }
  void GetFormatPointer(const CAnimSource*& ptr) const { ptr = &AsCAnimSource(); }
#if RSTL_CORRUPTION
  void GetFormatPointer(const CCompressedAnimSource*& ptr) const { ptr = &AsCCompressedAnimSource(); }
#endif
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
#if RSTL_CORRUPTION
  CVector3f x9c_;
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CVector3f x68_;
#else
  CVector3f x88_;
#endif

#endif
  SObjectTag mTag;
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CAllFormatsAnimSource, 0xb8)
#else
CHECK_SIZEOF(CAllFormatsAnimSource, RSTL_PRIME1 ? 0x7c : 0x9c)
#endif

template < typename T >
#if RSTL_CORRUPTION
inline TSubAnimTypeToken< T >::TSubAnimTypeToken(const TLockedToken< CAllFormatsAnimSource >& token)
: mToken(token) {
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
class TSubAnimTypeToken {
public:
  TSubAnimTypeToken(const TLockedToken< CAllFormatsAnimSource >& tok)
#else
inline TSubAnimTypeToken< T >::TSubAnimTypeToken(const TLockedToken< CAllFormatsAnimSource >& tok)
#endif
  : mToken(tok) {
#endif
    mToken->GetFormatPointer(mSource);
  }
#if RSTL_CORRUPTION

CFactoryFnReturn AnimSourceFactory(const SObjectTag& tag, const rstl::auto_ptr< uchar >& buffer,
                                   int size, const CVParamTransfer& param);
#else
#if RSTL_PRIME1
  const T* const& GetSource() const { return mSource; }
  const T* operator->() const { return mSource; }
  const T& operator*() const { return *mSource; }

private:
  TLockedToken< CAllFormatsAnimSource > mToken;
  const T* mSource;
};
#endif
#endif

#if !RSTL_CORRUPTION
RSTL_PRIME1_CONST CFactoryFnReturn AnimSourceFactory(const SObjectTag& tag, CInputStream& in,
                                                     const CVParamTransfer& param);
#endif

#endif // _CALLFORMATSANIMSOURCE
