#ifndef _CSCOPEDPROFILER
#define _CSCOPEDPROFILER

#include "rstl/types.h"

#include "Kyoto/Basics/CStopwatch.hpp"
#include "Kyoto/TFunctor.hpp"
#include "rstl/string.hpp"

class CScopedProfiler {
public:
  CScopedProfiler(const rstl::string& name, bool enabled);
  ~CScopedProfiler();
  static void BeginFrame();

private:
  CStopwatch mStopwatch;
  rstl::string mName;
  bool mEnabled;
};

// Guessed name
extern TFunctor3< const char*, const CStopwatch&, const CStopwatch& > gRenderProfileCallback;

#endif // _CSCOPEDPROFILER
