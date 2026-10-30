#ifndef _IOBJ
#define _IOBJ

#include "rstl/RstlVersions.h"
#include "types.h"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "rstl/auto_ptr.hpp"

extern const SObjectTag gkInvalidObjectTag;

class IObj {
public:
  virtual ~IObj() = 0;
};

inline IObj::~IObj() {}

class CObjOwnerDerivedFromIObjUntyped : public IObj {
public:
  virtual ~CObjOwnerDerivedFromIObjUntyped() = 0;
  template < typename T >
  CObjOwnerDerivedFromIObjUntyped(T* obj) : m_objPtr(obj) {}
  template < typename T >
  CObjOwnerDerivedFromIObjUntyped(const rstl::auto_ptr< T >& obj) : m_objPtr(obj.release()) {}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void* GetContents() const { return m_objPtr; }
#else
  void* GetContents() { return m_objPtr; }
#endif

protected:
  void* m_objPtr;
};

inline CObjOwnerDerivedFromIObjUntyped::~CObjOwnerDerivedFromIObjUntyped() {}

template < typename T >
class TObjOwnerDerivedFromIObj : public CObjOwnerDerivedFromIObjUntyped {
public:
  TObjOwnerDerivedFromIObj(T* obj) : CObjOwnerDerivedFromIObjUntyped(obj) {}
  ~TObjOwnerDerivedFromIObj() {
    if (Owned()) {
      delete Owned();
    }
  }

  T* Owned() { return static_cast< T* >(m_objPtr); }

#if RSTL_VERSION >= RSTL_R3IJ
  typedef rstl::auto_ptr< IObj > TOwnerPtr;
#else
  typedef rstl::auto_ptr< TObjOwnerDerivedFromIObj< T > > TOwnerPtr;
#endif

  static TOwnerPtr GetNewDerivedObject(T* obj) { return rs_new TObjOwnerDerivedFromIObj< T >(obj); }

  static TOwnerPtr GetNewDerivedObject(const rstl::auto_ptr< T >& obj) {
    return rs_new TObjOwnerDerivedFromIObj< T >(obj);
  }

private:
  TObjOwnerDerivedFromIObj(const rstl::auto_ptr< T >& obj) : CObjOwnerDerivedFromIObjUntyped(obj) {}
};

#endif // _IOBJ
