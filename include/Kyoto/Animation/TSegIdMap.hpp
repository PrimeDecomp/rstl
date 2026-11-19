#ifndef MP1_TSEGIDMAP_HPP
#define MP1_TSEGIDMAP_HPP
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"

#include "rstl/types.h"
#include "Kyoto/Animation/CCharAnimMemoryMetrics.hpp"
#include "Kyoto/Animation/CSegId.hpp"
#include "rstl/pair.hpp"

#include "rstl/reserved_vector.hpp"

template < typename T >
class TSegIdMap {
public:
  #if RSTL_PRIME1
  typedef char IndirectionType;
  #define INDIRECTION_TYPE_INVALID -1
  #else
  typedef CSegId IndirectionType;
  #define INDIRECTION_TYPE_INVALID CSegId::Null()
  #endif


// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1 && RSTL_VERSION < RSTL_GM8P_00
  inline
#endif
  TSegIdMap(uchar count);

  TSegIdMap(CInputStream& in);

  ~TSegIdMap();
  T& operator[](const CSegId& id) {
#if RSTL_PRIME1
    int index = mIndirectionMap[id.val()].second;
    return AccessElement(static_cast< uchar >(index));
#else
    return mNodes[mIndirectionMap[id.val()].second.val()];
#endif
  }

  const T& operator[](const CSegId& id) const {
    uint rawId = id.val();
#if RSTL_PRIME1
    return AccessElement(static_cast< uchar >(mIndirectionMap[rawId].second));
#else
    return AccessElement(mIndirectionMap[rawId].second.val());
#endif
  }

  bool ContainsDataFor(const CSegId& id) const {
    return mIndirectionMap[id.val()] != rstl::pair< IndirectionType, IndirectionType >(INDIRECTION_TYPE_INVALID, INDIRECTION_TYPE_INVALID);
  }
  int GetNumNodes() const { return mBoneCount; }

  T& AccessElement(int index) {
    RSTL_PRECONDITION(0 <= index && index < mBoneCount);
    return mNodes[index];
  }

  const T& AccessElement(int index) const {
    RSTL_PRECONDITION(0 <= index && index < mBoneCount);
    return mNodes[index];
  }

  void insert(const CSegId& id, const T& value);

private:
  char mBoneCount;
  char mCapacity;
  rstl::reserved_vector< rstl::pair< IndirectionType, IndirectionType >, 100 > mIndirectionMap;

  T* mNodes;
  IndirectionType mCurPrevBone;
};

template < typename T >
TSegIdMap< T >::TSegIdMap(CInputStream& in)
: mBoneCount(0)
, mCapacity(in.Get< uint >())
, mIndirectionMap(100, rstl::pair< IndirectionType, IndirectionType >(INDIRECTION_TYPE_INVALID, INDIRECTION_TYPE_INVALID))
, mNodes(reinterpret_cast< T* >(rs_new char[mCapacity * sizeof(T)]))
, mCurPrevBone(IF_RSTL_PRIME1_ELSE(0, CSegId::Null()))
{
  for (int i = 0; i < mCapacity; ++i) {
    CSegId seg(in);
    T node(in);
    insert(seg, node);
  }
#if !RSTL_PRIME1
  CCharAnimMemoryMetrics::AddToTotalSize(mCapacity, CCharAnimMemoryMetrics::kASS_Two);
#endif
}

template < typename T >
TSegIdMap< T >::TSegIdMap(uchar count)
: mBoneCount(0)
, mCapacity(count)
, mIndirectionMap(100, rstl::pair< IndirectionType, IndirectionType >(INDIRECTION_TYPE_INVALID, INDIRECTION_TYPE_INVALID))
, mNodes(nullptr)
, mCurPrevBone(IF_RSTL_PRIME1_ELSE(0, CSegId::Null()))
{
  mNodes = reinterpret_cast< T* >(rs_new uchar[count * sizeof(T)]);
#if !RSTL_PRIME1
  CCharAnimMemoryMetrics::AddToTotalSize(mCapacity, CCharAnimMemoryMetrics::kASS_Two);
#endif
}

template < typename T >
TSegIdMap< T >::~TSegIdMap() {
#if RSTL_PRIME1
  const rstl::pair< IndirectionType, IndirectionType >* const links = mIndirectionMap.data();
#endif
  CSegId id(mCurPrevBone);
  while (id != CSegId::Null()) {
#if !RSTL_PRIME1
    const int index = id.val();
#endif
    (*this)[id].~T();
#if RSTL_PRIME1
    id = CSegId(links[static_cast< int >(id.val())].first);
#else
    id = mIndirectionMap.begin()[index].first;
#endif
  }
#if !RSTL_PRIME1
  CCharAnimMemoryMetrics::SubtractFromTotalSize(mCapacity, CCharAnimMemoryMetrics::kASS_Two);
#endif
  delete[] reinterpret_cast< char* >(mNodes);
}

template < typename T >
void TSegIdMap< T >::insert(const CSegId& id, const T& value) {
  T* node = &mNodes[mBoneCount];
  uchar rawId = id.val();
  new (node) T(value);
  mIndirectionMap[rawId] = rstl::pair< IndirectionType, IndirectionType >(mCurPrevBone, IF_RSTL_PRIME1_ELSE(mBoneCount, CSegId(mBoneCount)));
  mCurPrevBone = IF_RSTL_PRIME1_ELSE(rawId, id);
  ++mBoneCount;
}

typedef TSegIdMap< uchar > TSegIdMapSizeCheck;
CHECK_SIZEOF(TSegIdMapSizeCheck, 0xd8)

#endif // MP1_TSEGIDMAP_HPP
