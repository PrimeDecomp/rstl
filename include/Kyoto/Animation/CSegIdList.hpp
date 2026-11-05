#ifndef _CSEGIDLIST
#define _CSEGIDLIST
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/CCharAnimMemoryMetrics.hpp"
#include "Kyoto/Animation/CSegId.hpp"
#include "rstl/vector.hpp"

class CInputStream;

class CSegIdList {
public:
  typedef rstl::vector< CSegId >::const_iterator const_iterator;
#if RSTL_PRIME1
  typedef rstl::vector< CSegId >::iterator iterator;
  typedef rstl::vector< CSegId >::size_type size_type;
  typedef rstl::vector< CSegId >::value_type value_type;
#endif

  CSegIdList(CInputStream& in);

  ~CSegIdList();

#if !RSTL_PRIME1
  int Size() const { return mList.size(); } // Guessed name, alternate guess GetCount
#endif
#if RSTL_VERSION == RSTL_G2MEAB
  // The prototype's assertion text spells the accessor size().
  int size() const { return mList.size(); }
#endif

  CSegId operator[](int idx) const { return mList[idx]; }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  size_type size() const { return mList.size(); }
  size_type Size() const { return mList.size(); } // Guessed name, alternate guess GetCount
  iterator begin() { return mList.begin(); }
#endif
  const_iterator begin() const { return mList.begin(); }
#if RSTL_PRIME1
  iterator end() { return mList.end(); }
#endif
  const_iterator end() const { return mList.end(); }

#if RSTL_PRIME1
private:
#endif

  rstl::vector< CSegId > mList; // Metaforce guess, alternate guess mSegIds or mSegList
};
#if !RSTL_PRIME1
#if RSTL_VERSION >= RSTL_R32J
CHECK_SIZEOF(CSegIdList, 0xc)
#else
CHECK_SIZEOF(CSegIdList, 0x10)
#endif
#endif

#if !RSTL_KYOTO_SEGIDLIST_OUT_OF_LINE_DTOR
inline CSegIdList::~CSegIdList() {
  CCharAnimMemoryMetrics::SubtractFromTotalSize(mList.capacity(), CCharAnimMemoryMetrics::kASS_Two);
}
#endif

#endif // _CSEGIDLIST
