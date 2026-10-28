#ifndef _PACKEDFLOAT
#define _PACKEDFLOAT

// Descriptive native label: the original namespace and signature are unproven. Loads a float
// stored unaligned in packed animation data; CVisibilityAnimationDataReader::GetTime is the
// only caller.

extern "C" float PackedFloat_LoadFloat32(const void* data);

#endif // _PACKEDFLOAT
