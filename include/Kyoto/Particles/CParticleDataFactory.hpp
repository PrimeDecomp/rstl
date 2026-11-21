#ifndef _CPARTICLEDATAFACTORY
#define _CPARTICLEDATAFACTORY
#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"

#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Particles/CElectricDescription.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#endif

#include "Kyoto/Particles/IElement.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/SObjectTag.hpp"
#endif
#include "Kyoto/TToken.hpp"

#if !RSTL_CORRUPTION
#include "Kyoto/SObjectTag.hpp"
#endif
#include "rstl/optional_object.hpp"
#include "rstl/vector.hpp"

#if !RSTL_CORRUPTION
class CInputStream;
#endif

class CElectricDescription;
class CGenDescription;
#if RSTL_CORRUPTION
class CInputStream;
#endif
class CModel;
class CSimplePool;
class CSwooshDescription;

class CParticleDataFactory {
public:
#if RSTL_CORRUPTION
  static CGenDescription* GetGeneratorDesc(CInputStream& in, CSimplePool* pool, CAssetId id);
  static CGenDescription* CreateGeneratorDescription(CInputStream&,
                                                     rstl::vector< CAssetId >& assets, CAssetId id,
                                                     CSimplePool* pool);
#else
  static CGenDescription* GetGeneratorDesc(CInputStream& in, CSimplePool* pool, uint id);
  static CGenDescription* CreateGeneratorDescription(CInputStream&, rstl::vector< uint >& assets,
                                                     uint id, CSimplePool* pool);
#endif
  static void LoadGPSMTokens(CGenDescription* desc);
  static bool CreateGPSM(CGenDescription* desc, CInputStream& in,
                         rstl::vector< CAssetId >& resources, CSimplePool* pool);
  static CUVElement* GetTextureElement(CInputStream& in, CSimplePool* resPool);
  static CColorElement* GetColorElement(CInputStream& in);
  static CModVectorElement* GetModVectorElement(CInputStream& in);
  static CEmitterElement* GetEmitterElement(CInputStream& in);
  static CVectorElement* GetVectorElement(CInputStream& in);
  static CRealElement* GetRealElement(CInputStream& in);
  static CIntElement* GetIntElement(CInputStream& in);

  static float GetReal(CInputStream& in);
  static int GetInt(CInputStream& in);
  static bool GetBool(CInputStream& in);
  static int GetBitflag(CInputStream& in);

  static FourCC GetClassID(CInputStream& in);
  static rstl::optional_object< TToken< CModel > > GetModel(CInputStream& in, CSimplePool* pool);
  static rstl::optional_object< TToken< CGenDescription > >
  GetChildGeneratorDesc(CInputStream& in, CSimplePool* pool,
                        const rstl::vector< CAssetId >& resources);
  static rstl::optional_object< TToken< CGenDescription > >
  GetChildGeneratorDesc(CAssetId id, CSimplePool* pool, const rstl::vector< CAssetId >& resources);
  static rstl::optional_object< TToken< CSwooshDescription > >
  GetSwooshGeneratorDesc(CInputStream& in, CSimplePool* pool);
  static rstl::optional_object< TToken< CElectricDescription > >
  GetElectricGeneratorDesc(CInputStream& in, CSimplePool* pool);
};

#endif // _CPARTICLEDATAFACTORY
