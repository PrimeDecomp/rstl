#ifndef _CRESFACTORY
#define _CRESFACTORY
#include "rstl/RstlVersions.h"

#include "types.h"
#include "rstl/auto_ptr.hpp"
#include "rstl/list.hpp"
#include "rstl/map.hpp"
#include "rstl/pair.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/CResLoader.hpp"
#include "Kyoto/CVParamTransfer.hpp"
#if RSTL_PRIME1
#include "Kyoto/IFactory.hpp"

struct z_stream_s;
#endif

class CDvdRequest;

#if !RSTL_PRIME1
class IFactory {
public:
  virtual ~IFactory() = 0;
  virtual rstl::auto_ptr< IObj > Build(const SObjectTag&, const CVParamTransfer&) = 0;
  virtual void BuildAsync(const SObjectTag&, const CVParamTransfer&, IObj**) = 0;
  virtual void CancelBuild(const SObjectTag&) = 0;
  virtual bool CanBuild(const SObjectTag&) = 0;
  virtual const SObjectTag* GetResourceIdByName(const char* name) const = 0;
};

inline IFactory::~IFactory() {}
#endif

class CResFactory : public IFactory {
public:
#if !RSTL_PRIME1
  struct SLoadingData {
    SObjectTag mTag;
    rstl::auto_ptr< CDvdRequest > mDvdReq;
    IObj** mTarget;
    rstl::auto_ptr< uchar > mBuffer;
    uint mCompressedPos;
    rstl::auto_ptr< uchar > mDecompBuffer;
    uint mDecompressedPos;
    int mSize;
    CResLoader::ECompressionType mCompression;
    CVParamTransfer mParams;

    SLoadingData(const SObjectTag& tag, CDvdRequest* request, IObj** target,
                 const rstl::auto_ptr< uchar >& buffer, int size,
                 CResLoader::ECompressionType compression, const CVParamTransfer& params);
    ~SLoadingData();
    bool PumpDecompression(uint time);
  };

  CResFactory();
#endif

  ~CResFactory() override;
  rstl::auto_ptr< IObj > Build(const SObjectTag&, const CVParamTransfer&) override;
  void BuildAsync(const SObjectTag&, const CVParamTransfer&, IObj**) override;
  void CancelBuild(const SObjectTag&) override;

  bool CanBuild(const SObjectTag& tag) override { return mResLoader.ResourceExists(tag); }

  const SObjectTag* GetResourceIdByName(const char* name) const override {
    return mResLoader.GetResourceIdByName(name);
  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  struct SLoadingData {
    SObjectTag mTag;
    rstl::auto_ptr< CDvdRequest > mDvdReq;
    IObj** mTarget;
    rstl::auto_ptr< uchar > mBuffer;
    rstl::auto_ptr< uchar > mDecompBuffer;
    rstl::auto_ptr< z_stream_s > mZip;
    int mSize;
    CResLoader::ECompressionType mCompression;
    CVParamTransfer mParams;

    SLoadingData(const SObjectTag& tag, CDvdRequest* request, IObj** target, void* buffer, int size,
                 CResLoader::ECompressionType compression, const CVParamTransfer& params);
    ~SLoadingData();
    bool PumpDecompression(uint time);
  };

  CResFactory();

  FourCC GetResourceTypeById(CAssetId id) { return GetResLoader().GetResourceTypeById(id); }

  rstl::vector< rstl::pair< rstl::string, SObjectTag > > GetResourceIdToNameList() const;

  uint ResourceSize(const SObjectTag& tag) { return mResLoader.ResourceSize(tag); }
#else
  uint ResourceSize(const SObjectTag& tag) const { return mResLoader.ResourceSize(tag); }
#endif

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
  void AsyncIdle(uint time, const bool);
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
#else
  FourCC GetResourceTypeById(CAssetId id) { return GetResLoader().GetResourceTypeById(id); }
  rstl::vector< rstl::pair< rstl::string, SObjectTag > > GetResourceIdToNameList() const;
#endif

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
#endif
CHECK_SIZEOF(CResFactory, RSTL_PRIME1 ? 0xc8 : 0xe0)

extern CResFactory* gpResourceFactory;

#endif // _CRESFACTORY
