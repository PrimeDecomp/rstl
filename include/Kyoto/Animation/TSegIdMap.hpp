#ifndef MP1_TSEGIDMAP_HPP
#define MP1_TSEGIDMAP_HPP
#include "rstl/RstlVersions.h"

#include "types.h"
#include "Kyoto/Animation/CCharAnimMemoryMetrics.hpp"
#include "Kyoto/Animation/CSegId.hpp"
#include "rstl/pair.hpp"

#include "rstl/reserved_vector.hpp"

template < typename T >
class TSegIdMap {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_GM8P_00
  TSegIdMap(uchar count);
#else
  inline TSegIdMap(uchar count);
#endif
#else
  TSegIdMap(uchar count);
#endif

  TSegIdMap(CInputStream& in);

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  T& operator[](const CSegId& id) {
    int index = mIndirectionMap[id.val()].second;
    return AccessElement(static_cast< uchar >(index));
  }
#else
  ~TSegIdMap();

  T& operator[](const CSegId& id) { return mNodes[mIndirectionMap[id.val()].second.val()]; }
#endif

  const T& operator[](const CSegId& id) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    uint rawId = id.val();
    return AccessElement(static_cast< uchar >(mIndirectionMap[rawId].second));
#else
    return mNodes[mIndirectionMap[id.val()].second.val()];
#endif

  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  bool ContainsDataFor(const CSegId& id) const {
    return mIndirectionMap[id.val()] != rstl::pair< char, char >(-1, -1);
  }
#else
  int GetNumNodes() const { return mBoneCount; }
#endif

#if RSTL_PRIME1
  T& AccessElement(int index) {
    T* ptr = mNodes + index;
    return *ptr;
  }
#endif

  const T& AccessElement(int index) const { return mNodes[index]; }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  ~TSegIdMap();
#else
  bool ContainsDataFor(const CSegId& id) const {
    return mIndirectionMap[id.val()] !=
           rstl::pair< CSegId, CSegId >(CSegId::Null(), CSegId::Null());
  }
#endif

  void insert(const CSegId& id, const T& value);

private:
  char mBoneCount;
  char mCapacity;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::reserved_vector< rstl::pair< char, char >, 100 > mIndirectionMap;
#else
  rstl::reserved_vector< rstl::pair< CSegId, CSegId >, 100 > mIndirectionMap;
#endif

  T* mNodes;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  char mCurPrevBone;
#else
  CSegId mCurPrevBone;
#endif

};
#if !RSTL_PRIME1
typedef TSegIdMap< uchar > TSegIdMapSizeCheck;
CHECK_SIZEOF(TSegIdMapSizeCheck, 0xd8)
#endif

template < typename T >
TSegIdMap< T >::TSegIdMap(CInputStream& in)
: mBoneCount(0)
, mCapacity(in.Get< uint >())
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, mIndirectionMap(100, rstl::pair< char, char >(-1, -1))
#else
, mIndirectionMap(100, rstl::pair< CSegId, CSegId >(CSegId::Null(), CSegId::Null()))
#endif

, mNodes(reinterpret_cast< T* >(rs_new char[mCapacity * sizeof(T)]))
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, mCurPrevBone(0) {
  for (int i = 0; i < mCapacity; i++) {
#else
, mCurPrevBone(CSegId::Null()) {
  for (int i = 0; i < mCapacity; ++i) {
#endif

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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, mIndirectionMap(100, rstl::pair< char, char >(-1, -1))
#else
, mIndirectionMap(100, rstl::pair< CSegId, CSegId >(CSegId::Null(), CSegId::Null()))
#endif

, mNodes(nullptr)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, mCurPrevBone(0) {
#else
, mCurPrevBone(CSegId::Null()) {
#endif

  mNodes = reinterpret_cast< T* >(rs_new uchar[count * sizeof(T)]);
#if !RSTL_PRIME1
  CCharAnimMemoryMetrics::AddToTotalSize(mCapacity, CCharAnimMemoryMetrics::kASS_Two);
#endif

}

template < typename T >
TSegIdMap< T >::~TSegIdMap() {
#if RSTL_PRIME1
  const rstl::pair< char, char >* const links = mIndirectionMap.data();
#endif

  CSegId id(mCurPrevBone);
  while (id != CSegId::Null()) {
#if !RSTL_PRIME1
    const int index = id.val();
#endif

    (*this)[id].~T();
// TODO: verify that this Prime/Echoes difference is real
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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  mIndirectionMap[rawId] = rstl::pair< char, char >(mCurPrevBone, mBoneCount);
  mCurPrevBone = rawId;
#else
  mIndirectionMap[rawId] = rstl::pair< CSegId, CSegId >(mCurPrevBone, CSegId(mBoneCount));
  mCurPrevBone = id;
#endif

  ++mBoneCount;
}

#if RSTL_PRIME1
typedef TSegIdMap< uchar > unk_TSegIdMap;
CHECK_SIZEOF(unk_TSegIdMap, 0xd8)
#endif

#endif // MP1_TSEGIDMAP_HPP
