#ifndef _CSFXHANDLE
#define _CSFXHANDLE

#include "rstl/RstlVersions.h"
#include "types.h"

class CSfxHandle {
public:
  CSfxHandle() : mID(0) {}
  CSfxHandle(uint value);

#if RSTL_KYOTO_CONST_SCALAR_RETURNS
  int GetIndex() const { return mID & 0xFFF; }
#else
  uint GetIndex() const { return mID & 0xFFF; }
#endif
  static CSfxHandle NullHandle() { return CSfxHandle(); }
#if RSTL_KYOTO_CONST_SCALAR_RETURNS
  void operator=(const CSfxHandle& other) { mID = other.mID; }
  const bool operator==(const CSfxHandle& other) const { return mID == other.mID; }
  const bool operator!=(const CSfxHandle& other) const { return mID != other.mID; }
#else
  bool operator==(const CSfxHandle& other) const { return mID == other.mID; }
  bool operator!=(const CSfxHandle& other) const { return mID != other.mID; }
#endif
  operator bool() const { return mID != 0; }
  void Clear() { mID = 0; }

private:
  uint mID;
  static uint mRefCount;
};
CHECK_SIZEOF(CSfxHandle, 0x4)

#endif // _CSFXHANDLE
