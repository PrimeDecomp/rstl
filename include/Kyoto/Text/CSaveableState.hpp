#ifndef _CSAVEABLESTATE
#define _CSAVEABLESTATE
#include "rstl/RstlVersions.h"

#include "Kyoto/Text/CDrawStringOptions.hpp"
#include "Kyoto/Text/CRasterFont.hpp"
#include "Kyoto/Text/CTextColor.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/vector.hpp"

class CSaveableState {
public:
  CSaveableState();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  bool IsFinishedLoading();
#else
  bool IsFinishedLoading() const;
#endif

  CDrawStringOptions& GetOptions() { return mDrawStringOptions; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::optional_object< TToken< CRasterFont > >& GetFont() { return mFont; }
#else
  TToken< CRasterFont >& GetFont() { return *mFont; }
#endif

  void SetFont(const TToken< CRasterFont >& font) { mFont = font; }
  rstl::vector< CTextColor >& GetColors() { return mColors; }
  rstl::vector< bool >& GetOverride() { return mColorOverrides; }
#if !RSTL_PRIME1
  void SetLineSpacing(float spacing) { mLineSpacing = spacing; }
  void SetLineExtraSpace(int spacing) { mExtraLineSpacing = spacing; }
#endif

  float GetLineSpacing() const { return mLineSpacing; }
#if RSTL_PRIME1
  void SetLineSpacing(float spacing) { mLineSpacing = spacing; }
#endif

  int GetLineExtraSpacing() const { return mExtraLineSpacing; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void SetLineExtraSpace(int spacing) { mExtraLineSpacing = spacing; }
  void SetWordWrapping(const bool wrap) { mEnableWordWrap = wrap; }
#else
  void SetWordWrapping(bool wrap) { mEnableWordWrap = wrap; }
#endif

  bool IsWordWrapping() const { return mEnableWordWrap; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void SetJustification(EJustification just) { mJust = just; }
  void SetVerticalJustification(EVerticalJustification just) { mVjust = just; }
#else
  void SetJustification(EJustification justification) { mJust = justification; }
#endif

  EJustification GetJustification() const { return mJust; }
#if !RSTL_PRIME1
  void SetVerticalJustification(EVerticalJustification justification) { mVjust = justification; }
#endif

  EVerticalJustification GetVerticalJustification() const { return mVjust; }

#if !RSTL_PRIME1
private:
#endif

  CDrawStringOptions mDrawStringOptions;
  rstl::optional_object< TToken< CRasterFont > > mFont;
  rstl::vector< CTextColor > mColors;
  rstl::vector< bool > mColorOverrides;
  float mLineSpacing;
  int mExtraLineSpacing;
  bool mEnableWordWrap;
  EJustification mJust;
  EVerticalJustification mVjust;
};

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CHECK_SIZEOF(CSaveableState,
             (RSTL_VERSION >= RSTL_GM8P_00 ? 0x8c : 0x88))
#else
CHECK_SIZEOF(CSaveableState, 0x8c)
#endif

#endif // _CSAVEABLESTATE
