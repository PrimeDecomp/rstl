#ifndef _CVISIBILITYANIMATIONDATALIST
#define _CVISIBILITYANIMATIONDATALIST

#include "rstl/static_assert.hpp"
#include "rstl/types.h"

class CCharAnimTime;
class CInputStream;
class CVisibilityAnimationDataReader;

class CVisibilityAnimationDataList {
public:
  explicit CVisibilityAnimationDataList(CInputStream& in);
  CVisibilityAnimationDataList(const void* data, uint size);
  ~CVisibilityAnimationDataList();

  // Guessed name
  const CVisibilityAnimationDataReader* GetRecordByIndex(uint index) const;
  // Guessed name
  int FindRecordIndexAtTime(const CCharAnimTime& time, int startIndex) const;

private:
  static uchar* ReadOwnedBuffer(CInputStream& in, uint size); // Guessed name

  ushort mNumRecords; // Guessed name
  uchar mRecordSize;  // Guessed name
  bool mOwnsData;     // Guessed name
  uchar* mData;       // Guessed name
};
CHECK_SIZEOF(CVisibilityAnimationDataList, 0x8)

#endif // _CVISIBILITYANIMATIONDATALIST
