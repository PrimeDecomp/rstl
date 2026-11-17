#ifndef _SAREASURFACE
#define _SAREASURFACE

#include "rstl/static_assert.hpp"
#include "Kyoto/Math/CAABox.hpp"

class CInputStream;

// Guessed name. Entries describe area surfaces, with entry zero reserved.
struct SAreaSurface {
  explicit SAreaSurface(CInputStream& in);

  int GetSurfaceGroupIndex() const { return mSurfaceGroupIndex; } // Guessed name

  CAABox mBounds;
  short mModelIndex;
  short mSurfaceGroupIndex; // Guessed name; selects a model's group of surface indices.
  short x1c_;
  short x1e_;
};
CHECK_SIZEOF(SAreaSurface, 0x20)

#endif // _SAREASURFACE
