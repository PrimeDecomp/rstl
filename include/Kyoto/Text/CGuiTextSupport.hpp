#ifndef _CGUITEXTSUPPORT
#define _CGUITEXTSUPPORT
#include "rstl/RstlVersions.h"

#include "rstl/static_assert.hpp"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"

#endif
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CVector2i.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/SObjectTag.hpp"
#endif
#include "Kyoto/Text/CTextExecuteBuffer.hpp"
#include "Kyoto/Text/CTextRenderBuffer.hpp"
#if !RSTL_CORRUPTION
#include "Kyoto/SObjectTag.hpp"
#endif
#include "rstl/list.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#if RSTL_CORRUPTION
#include "rstl/single_ptr.hpp"
#endif
#include "rstl/string.hpp"

class CGuiTextProperties {
  friend class CGuiTextSupport;

public:
  CGuiTextProperties(bool wordWrap, EJustification justification,
                     EVerticalJustification verticalJustification,
                     const rstl::vector< rstl::pair< CAssetId, CAssetId > >* textureMap = nullptr);

private:
  bool mWordWrap;
  EJustification mJustification;
  EVerticalJustification mVerticalJustification;
  const rstl::vector< rstl::pair< CAssetId, CAssetId > >* mTextureMap;
};

class CGuiTextSupport {
public:
  CGuiTextSupport(CAssetId font, int extentX, int extentY, const CGuiTextProperties& properties,
                  const CColor& fontColor, const CColor& outlineColor, const CColor& geometryColor,
                  CSimplePool* pool);
  ~CGuiTextSupport();
  void SetText(const rstl::wstring& text, bool multipage = false);
  void SetText(const rstl::string& text, bool multipage = false);
  void AddText(const rstl::wstring& text);
  void SetFontColor(const CColor& color);
  void SetOutlineColor(const CColor& color);
  void SetGeometryColor(const CColor& color);
#if !RSTL_CORRUPTION
  const CColor& GetGeometryColor() const { return mGeometryColor; }
#endif
  void SetControlTXTRMap(const rstl::vector< rstl::pair< CAssetId, CAssetId > >* textureMap);
  void SetWordWrap(bool wordWrap);
#if !RSTL_CORRUPTION
  void SetImageBaseline(bool baseline);
  void SetJustification(EJustification justification);
#endif
  void SetVerticalJustification(EVerticalJustification justification);
#if RSTL_CORRUPTION
  void SetImageBaseline(bool baseline);
#endif
  void SetTypeWriteEffectOptions(bool enable, float fadeTime, float rate);
  void Update(float dt);
  void Render() const;
  void CheckAndRebuildTextBuffer() const;
  bool CheckAndRebuildRenderBuffer() const;
  void ClearRenderBuffer();
  bool GetIsTextSupportFinishedLoading() const;
  float GetTotalAnimationTime() const;
  float GetCurTime() const { return mCurrentTime; }
  void SetCurTime(float time) { mCurrentTime = time; }
#if RSTL_CORRUPTION
  const CColor& GetGeometryColor() const { return mGeometryColor; }
  void SetExtraLineSpacing(int spacing) { mExtraLineSpacing = spacing; } // Guessed name
  void SetExtraCharacterSpacing(int spacing) { mExtraCharacterSpacing = spacing; } // Guessed name
  void SetLineSpacingScale(float scale) { mLineSpacingScale = scale; } // Guessed name
#endif
  float GetNumCharactersPrinted() const;
#if !RSTL_CORRUPTION
  float GetNumCharsTotal() const;
#endif
  float GetCurrentAnimationOverAge() const;
  const rstl::wstring& GetText() const { return mText; }
  int GetTotalPageCount();
  void SetPage(int page);
  CTextRenderBuffer* GetCurrentPageRenderBuffer() const;
  const rstl::pair< CVector2i, CVector2i >& GetBounds();

  int GetTextBoundingWidth() const { return mExtentX; }
  int GetTextBoundingHeight() const { return mExtentY; }
  CAssetId GetFontID() const { return mFontId; }
#if !RSTL_CORRUPTION

#if RSTL_GUI_TEXT_LINE_SPACING
  void SetTextSpacing(float lineSpacing, int lineExtraSpace, int characterExtraSpace) {
    mLineSpacing = lineSpacing;
    mExtraLineSpacing = lineExtraSpace;
    mExtraCharacterSpacing = characterExtraSpace;
  }
#endif
#endif

  void SetExtentX(int extent) {
    mExtentX = extent;
    ClearRenderBuffer();
  }

  void SetExtentY(int extent) {
    mExtentY = extent;
    ClearRenderBuffer();
  }

private:
#if RSTL_CORRUPTION
  // Guessed name
  struct STextBufferState {
    STextBufferState() : mBounds(CVector2i(0, 0), CVector2i(0, 0)) {}

    CTextExecuteBuffer mExecuteBuffer;
    rstl::optional_object< CTextRenderBuffer > mRenderBuffer;
    rstl::vector< CToken > mAssets;
    rstl::pair< CVector2i, CVector2i > mBounds;
    rstl::list< CTextRenderBuffer > mPages;
  };

#endif
  bool _GetIsTextSupportFinishedLoading() const;

  CSimplePool* mPool;
#if !RSTL_CORRUPTION
  mutable CTextExecuteBuffer mExecuteBuffer;
#endif
  rstl::wstring mText;
  float mCurrentTimeMod900;
  CGuiTextProperties mProperties;
  CColor mFontColor;
  CColor mOutlineColor;
  CColor mGeometryColor;
  bool mImageBaseline;
  int mExtraCharacterSpacing;
  int mExtraLineSpacing;
#if RSTL_CORRUPTION
  float mLineSpacingScale; // Guessed name
#else
#if RSTL_GUI_TEXT_LINE_SPACING
  float mLineSpacing;
#endif
#endif
  int mExtentX;
  int mExtentY;
  float mCurrentTime;
  rstl::vector< rstl::pair< float, int > > mPrimitiveStartTimes;
  bool mTypewriterEnabled;
  float mCharacterFadeTime;
  float mCharacterRate;
  CAssetId mFontId;
#if RSTL_CORRUPTION
  rstl::single_ptr< STextBufferState > mTextBufferState;
  rstl::optional_object< CToken > mFont;
#else
  mutable rstl::optional_object< CTextRenderBuffer > mRenderBuffer;
  mutable rstl::vector< CToken > mAssets;
  rstl::optional_object< TToken< CRasterFont > > mFont;
  mutable rstl::pair< CVector2i, CVector2i > mBounds;
  mutable rstl::list< CTextRenderBuffer > mPages;
#endif
  int mPageCounter;
  bool mMultipage;
};

CHECK_SIZEOF(CGuiTextProperties, 0x10)
#if RSTL_CORRUPTION
CHECK_SIZEOF(CGuiTextSupport, 0x90)
#else
CHECK_SIZEOF(CGuiTextSupport,
             RSTL_CORRUPTION ? 0xd18 : RSTL_GUI_TEXT_LINE_SPACING ? (RSTL_ECHOES ? 0xd0c : 0xcdc) : 0xd08)
#endif

#endif // _CGUITEXTSUPPORT
