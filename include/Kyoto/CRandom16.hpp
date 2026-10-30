#ifndef _CRANDOM16
#define _CRANDOM16

#include "types.h"

#include "rstl/RstlVersions.h"

class CRandom16;
class CGlobalRandom {
  static CGlobalRandom* gCurrentGlobalRandom;

public:
  CGlobalRandom(CRandom16& rnd);
  ~CGlobalRandom();

private:
  CRandom16& mRandom;
  bool mIsFirst;
  CGlobalRandom* mPrev;
};

class CRandom16 {
  friend class CGlobalRandom;
  static CRandom16* gRandomNumber;
  static void _SetRandomNumber(CRandom16* rnd);

public:
  CRandom16(uint seed = 99);
  void SetSeed(uint seed);
#if RSTL_KYOTO_CONST_SCALAR_RETURNS
  int Range(int min, int max);
  float Range(float min, float max);
  int Next();
  const float Float();

  uint GetSeed() const { return mSeed; }
#else
  uint GetSeed() const { return mSeed; }
  int Range(int min, int max);
  float Range(const float min, const float max);
  int Next();
  float Float();

#endif
  static CRandom16* GetRandomNumber();

private:
  uint mSeed;
};

#endif // _CRANDOM16
