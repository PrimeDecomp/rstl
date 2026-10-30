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
  bool IsFinishedLoading()
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
      const
#endif
      ;

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
#endif
  void SetWordWrapping(RSTL_PRIME1_CONST bool wrap) { mEnableWordWrap = wrap; }

  bool IsWordWrapping() const { return mEnableWordWrap; }
  void SetJustification(EJustification just) { mJust = just; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void SetVerticalJustification(EVerticalJustification just) { mVjust = just; }
#endif

  EJustification GetJustification() const { return mJust; }
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
  void SetVerticalJustification(EVerticalJustification just) { mVjust = just; }
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

CHECK_SIZEOF(CSaveableState, (RSTL_VERSION >= RSTL_GM8P_00 ? 0x8c : 0x88))

#endif // _CSAVEABLESTATE
