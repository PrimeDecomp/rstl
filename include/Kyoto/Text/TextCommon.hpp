#ifndef _TEXTCOMMON
#define _TEXTCOMMON
#include "rstl/RstlVersions.h"

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#include "types.h"
#else
#include "EchoesNamespace.h"
#endif

enum EColorType {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  kCT_Main,
#else
  kCT_Main = 0,
#endif

  kCT_Outline,
  kCT_Geometry,
  kCT_Foreground,
  kCT_Background,
};

enum ETextDirection {
  kTD_Horizontal,
  kTD_Vertical,
};

enum EJustification {
#if !RSTL_PRIME1
  kJustification_Invalid = -1, // Guessed name
#endif

  kJustification_Left = 0,
  kJustification_Center,
  kJustification_Right,
  kJustification_Full,
  kJustification_NLeft, // Metaforce guess, alternate guess NarrowLeft
  kJustification_NCenter, // Metaforce guess, alternate guess NarrowCenter
  kJustification_NRight, // Metaforce guess, alternate guess NarrowRight
  kJustification_LeftMono, // Metaforce guess, alternate guess EvenLeft
  kJustification_CenterMono, // Metaforce guess, alternate guess EvenCenter
  kJustification_RightMono, // Metaforce guess, alternate guess EvenRight
#if !RSTL_PRIME1
  kJustification_Count,
  kJustification_FirstValid = kJustification_Left,
  kJustification_LastValid = kJustification_RightMono,
#endif

};

enum EVerticalJustification {
#if !RSTL_PRIME1
  kVerticalJustification_Invalid = -1, // Guessed name
#endif

  kVerticalJustification_Top = 0,
  kVerticalJustification_Center,
  kVerticalJustification_Bottom,
  kVerticalJustification_Full,
  kVerticalJustification_NTop, // Metaforce guess, alternate guess NarrowTop
  kVerticalJustification_NCenter, // Metaforce guess, alternate guess NarrowCenter
  kVerticalJustification_NBottom, // Metaforce guess, alternate guess NarrowBottom
  kVerticalJustification_TopMono, // Metaforce guess, alternate guess EvenTop
  kVerticalJustification_CenterMono, // Metaforce guess, alternate guess EvenCenter
  kVerticalJustification_BottomMono, // Guessed name, Metaforce has RightMono, alternate guess EvenBottom
#if !RSTL_PRIME1
  kVerticalJustification_Count,
  kVerticalJustification_FirstValid = kVerticalJustification_Top,
  kVerticalJustification_LastValid = kVerticalJustification_BottomMono,
#endif

};

#endif // _TEXTCOMMON
