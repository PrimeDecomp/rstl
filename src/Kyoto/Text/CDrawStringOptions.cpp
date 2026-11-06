#include "rstl/RstlVersions.h"
#include "Kyoto/Text/CDrawStringOptions.hpp"

static const uint col = 0;

CDrawStringOptions::CDrawStringOptions()
: mDirection(kTD_Horizontal)
#if RSTL_CORRUPTION
, mColors(16, col)
#else
, mColors(col)
#endif
#if RSTL_VERSION >= RSTL_GM8P_00
, mExtraCharacterSpacing(0)
#endif
{}

