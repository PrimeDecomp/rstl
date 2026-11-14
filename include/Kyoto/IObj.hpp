#ifndef _IOBJ
#define _IOBJ

#include "rstl/RstlVersions.h"
#include "rstl/types.h"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "rstl/auto_ptr.hpp"

extern const SObjectTag gkInvalidObjectTag;

class IObj {
public:
#if RSTL_KYOTO_IOBJ_INLINE_DTOR
  virtual ~IObj() {}
#else
  virtual ~IObj() = 0;
#endif
};

#if !RSTL_KYOTO_IOBJ_INLINE_DTOR
inline IObj::~IObj() {}
#endif

class CObjOwnerDerivedFromIObjUntyped : public IObj {
public:
#if RSTL_KYOTO_IOBJ_INLINE_DTOR
  ~CObjOwnerDerivedFromIObjUntyped() {}
#else
  virtual ~CObjOwnerDerivedFromIObjUntyped() = 0;
#endif
  template < typename T >
  CObjOwnerDerivedFromIObjUntyped(T* obj) : m_objPtr(obj) {}
  template < typename T >
  CObjOwnerDerivedFromIObjUntyped(const rstl::auto_ptr< T >& obj) : m_objPtr(obj.release()) {}

  void* GetContents() RSTL_PRIME1_CONST { return m_objPtr; }

protected:
  void* m_objPtr;
};

#if !RSTL_KYOTO_IOBJ_INLINE_DTOR
inline CObjOwnerDerivedFromIObjUntyped::~CObjOwnerDerivedFromIObjUntyped() {}
#endif

// The source file name its allocation tags report; a unit built from a source that spells it
// differently defines this before including the header.
#ifndef RS_TTOKEN_FILE
#define RS_TTOKEN_FILE "TToken.h"
#endif

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

  static TOwnerPtr GetNewDerivedObject(T* obj) {
    return rs_new_line_in(RS_TTOKEN_FILE, 76) TObjOwnerDerivedFromIObj< T >(obj);
  }

  static TOwnerPtr GetNewDerivedObject(const rstl::auto_ptr< T >& obj) {
    return rs_new_line_in(RS_TTOKEN_FILE, 70) TObjOwnerDerivedFromIObj< T >(obj);
  }

private:
  TObjOwnerDerivedFromIObj(const rstl::auto_ptr< T >& obj) : CObjOwnerDerivedFromIObjUntyped(obj) {}
};

#endif // _IOBJ
