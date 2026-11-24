#include "Kyoto/TGameTypes.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

#if !RSTL_CORRUPTION
const TEditorId kInvalidEditorId = TEditorId(-1);
const TUniqueId kInvalidUniqueId = TUniqueId(-1, -1);
const TAreaId kInvalidAreaId = TAreaId(-1);
const TEditorId kUnkId = TEditorId(-1);
const uint kInvalidPlayerIndex = uint(-1);
const uint kFirstPlayerIndex = 0;
#endif

TEditorId::TEditorId(CInputStream& in) : value(in.Get< uint >()) {}
