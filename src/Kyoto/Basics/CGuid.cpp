#include "Kyoto/Basics/CGuid.hpp"

#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"

CGuid::CGuid(CInputStream& in) { in.Get(mData, sizeof(mData)); }

void CGuid::PutTo(COutputStream& out) const { out.DoPut(mData, sizeof(mData)); }
