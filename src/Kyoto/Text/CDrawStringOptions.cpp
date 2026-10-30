#include "rstl/RstlVersions.h"
#include "Kyoto/Text/CDrawStringOptions.hpp"

static const uint col = 0;

CDrawStringOptions::CDrawStringOptions()
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
: mDirection(kTD_Horizontal)
, mColors(col)
#if RSTL_VERSION >= VERSION_GM8P_00
, mExtraCharacterSpacing(0)
#endif
{}
#else
: mDirection(kTD_Horizontal), mColors(col), mCharacterExtraSpace(0) {}
#endif

