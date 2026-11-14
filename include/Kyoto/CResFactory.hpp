#ifndef _CRESFACTORY
#define _CRESFACTORY
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"

#include "rstl/auto_ptr.hpp"
#include "rstl/list.hpp"
#include "rstl/map.hpp"
#include "rstl/pair.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"
#include "rstl/types.h"


#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/CResLoader.hpp"
#include "Kyoto/CVParamTransfer.hpp"
#include "Kyoto/IFactory.hpp"

#if RSTL_PRIME1
struct z_stream_s;
#endif

class CDvdRequest;

class CResFactory : public IFactory {
public:
  struct SLoadingData {
    SObjectTag mTag;
    rstl::auto_ptr< CDvdRequest > mDvdReq;
    IObj** mTarget;
    rstl::auto_ptr< uchar > mBuffer;
#if !RSTL_PRIME1
    uint mCompressedPos;
#endif
    rstl::auto_ptr< uchar > mDecompBuffer;
#if RSTL_PRIME1
    rstl::auto_ptr< z_stream_s > mZip;
#else
    uint mDecompressedPos;
#endif
    int mSize;
    CResLoader::ECompressionType mCompression;
    CVParamTransfer mParams;

    SLoadingData(const SObjectTag& tag, CDvdRequest* request, IObj** target,
                 CFactoryMgr::MemoryBufferPtr buffer, int size,
                 CResLoader::ECompressionType compression, const CVParamTransfer& params);
    ~SLoadingData();
    bool PumpDecompression(uint time);
  };

  CResFactory();

  ~CResFactory() override;
  rstl::auto_ptr< IObj > Build(const SObjectTag&, const CVParamTransfer&) override;
  void BuildAsync(const SObjectTag&, const CVParamTransfer&, IObj**) override;
  void CancelBuild(const SObjectTag&) override;

  bool CanBuild(const SObjectTag& tag) override { return mResLoader.ResourceExists(tag); }

  const SObjectTag* GetResourceIdByName(const char* name) const override {
    return mResLoader.GetResourceIdByName(name);
  }

  uint ResourceSize(const SObjectTag& tag) RSTL_ECHOES_CONST {
    return mResLoader.ResourceSize(tag);
  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CDvdRequest* LoadResourceAsync(const SObjectTag& tag, void* buffer) {
    return mResLoader.LoadResourceAsync(tag, static_cast< char* >(buffer));
  }

  CInputStream* LoadNewResourceSync(const SObjectTag& tag, void* buffer) {
    CInputStream* const result = mResLoader.LoadNewResourceSync(tag, static_cast< char* >(buffer));
    return result;
  }

  void AsyncIdle(uint time);
#else
  void AsyncIdle(uint time, RSTL_KYOTO_SCALAR_PARAM_CONST bool);
#endif

  CResLoader& GetResLoader() { return mResLoader; }
  CFactoryMgr& GetFactoryMgr() { return mFactoryMgr; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::vector< CAssetId >* GetTagListForFile(const rstl::string& pak) const {
    return mResLoader.GetTagListForFile(pak);
  }

  CInputStream* LoadResourceFromMemorySync(const SObjectTag& tag, const void* extBuf) {
    return mResLoader.LoadResourceFromMemorySync(tag, extBuf);
  }
#endif
  FourCC GetResourceTypeById(CAssetId id) { return GetResLoader().GetResourceTypeById(id); }
  rstl::vector< rstl::pair< rstl::string, SObjectTag > > GetResourceIdToNameList() const;

private:
  typedef rstl::list< SLoadingData > LoadList;

  void AddToLoadList(const SLoadingData& data);
  void EraseFromLoadList(const LoadList::iterator& it);
  LoadList::iterator FindInLoadList(const SObjectTag& tag);
  rstl::auto_ptr< IObj > BuildSync(const SObjectTag& tag, const CVParamTransfer& params);
  bool PumpResource(const LoadList::iterator& it, uint time);

  CResLoader mResLoader;
  CFactoryMgr mFactoryMgr;
  LoadList mLoadList;
  rstl::map< SObjectTag, LoadList::iterator > mLoadMap;
  LoadList mCancelledList;
};
#if RSTL_PRIME1
NESTED_CHECK_SIZEOF(CResFactory, SLoadingData, 0x38)
#elif RSTL_KYOTO_ASSET_ID_CLASS
NESTED_CHECK_SIZEOF(CResFactory, SLoadingData, 0x48)
#endif
CHECK_SIZEOF(CResFactory, RSTL_PRIME1 ? 0xc8 : RSTL_KYOTO_BUFFER_FACTORIES ? 0xd8 : 0xe0)

#if RSTL_KYOTO_RES_FACTORY_INLINE_NAME_LIST
inline rstl::vector< rstl::pair< rstl::string, SObjectTag > >
CResFactory::GetResourceIdToNameList() const {
  return mResLoader.GetResourceIdToNameList();
}
#endif

extern CResFactory* gpResourceFactory;
extern int gResFactoryBuildLogMode; // Guessed name

#endif // _CRESFACTORY
