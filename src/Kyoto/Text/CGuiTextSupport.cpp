#include "rstl/RstlVersions.h"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Text/CGuiTextSupport.hpp"

#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Text/CInstruction.hpp"
#include "Kyoto/Text/CRasterFont.hpp"
#include "Kyoto/Text/CTextParser.hpp"
#include "Kyoto/Text/ScreenText.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "rstl/StringExtras.hpp"
#include "rstl/math.hpp"
#include "math.h"

CGuiTextProperties::CGuiTextProperties(
    bool wordWrap, EJustification justification, EVerticalJustification verticalJustification,
    const rstl::vector< rstl::pair< CAssetId, CAssetId > >* textureMap)
: mWordWrap(wordWrap)
, mJustification(justification)
, mVerticalJustification(verticalJustification)
, mTextureMap(textureMap) {}

CGuiTextSupport::CGuiTextSupport(CAssetId font, int extentX, int extentY,
                                 const CGuiTextProperties& properties, const CColor& fontColor,
                                 const CColor& outlineColor, const CColor& geometryColor,
                                 CSimplePool* pool)
: mPool(pool)
, mCurrentTimeMod900(0.f)
, mProperties(properties)
, mFontColor(fontColor)
, mOutlineColor(outlineColor)
, mGeometryColor(geometryColor)
, mImageBaseline(false)
, mExtraCharacterSpacing(0)
, mExtraLineSpacing(0)
#if RSTL_CORRUPTION
, mLineSpacingScale(0.f)
#else
#if RSTL_GUI_TEXT_LINE_SPACING
, mLineSpacing(0.f)
#endif
#endif
, mExtentX(extentX)
, mExtentY(extentY)
, mCurrentTime(0.f)
, mTypewriterEnabled(false)
, mCharacterFadeTime(0.1f)
, mCharacterRate(10.f)
, mFontId(font)
#if RSTL_CORRUPTION
, mTextBufferState(rs_new_line(91) STextBufferState)
#else
, mBounds(CVector2i(0, 0), CVector2i(0, 0))
#endif
, mPageCounter(0)
, mMultipage(false) {
  if (mFontId != kInvalidAssetId) {
#if RSTL_CORRUPTION
    mFont = pool->GetObj(SObjectTag('FONT', mFontId));
#else
    mFont = TToken< CRasterFont >(pool->GetObj(SObjectTag('FONT', mFontId)));
#endif
    mFont->Lock();
  }
}

CGuiTextSupport::~CGuiTextSupport() {}

bool CGuiTextSupport::GetIsTextSupportFinishedLoading() const {
  CheckAndRebuildRenderBuffer();
  return _GetIsTextSupportFinishedLoading();
}

bool CGuiTextSupport::_GetIsTextSupportFinishedLoading() const {
#if RSTL_CORRUPTION
  STextBufferState& state = *mTextBufferState.get();
  for (int i = 0; i < state.mAssets.size(); ++i) {
    if (!state.mAssets[i].IsLocked()) {
      state.mAssets[i].Lock();
#else
  for (int i = 0; i < mAssets.size(); ++i) {
    if (!mAssets[i].IsLocked()) {
      mAssets[i].Lock();
#endif
    }
#if RSTL_CORRUPTION
    if (!state.mAssets[i].IsLoaded()) {
#else
    if (!mAssets[i].IsLoaded()) {
#endif
      return false;
    }
  }
  if (mFont.valid()) {
    TToken< CRasterFont > font = *mFont;
    if (!font.IsLoaded()) {
      return false;
    }
    return font->IsFinishedLoading();
  }
#if RSTL_CORRUPTION
  if (!mFont.valid() && state.mAssets.size() == 0) {
#else
  if (!mFont.valid() && mAssets.size() == 0) {
#endif
    return false;
  }
  return true;
}

void CGuiTextSupport::SetText(const rstl::string& text, bool multipage) {
  const rstl::wstring wtext = CStringExtras::ConvertToUNICODE(text);
  SetText(wtext, multipage);
}

void CGuiTextSupport::SetText(const rstl::wstring& text, bool multipage) {
  if (mText != text) {
    mPrimitiveStartTimes.clear();
    mCurrentTime = 0.f;
    mText = text;
    ClearRenderBuffer();
    mMultipage = multipage;
    mPageCounter = 0;
  }
}

void CGuiTextSupport::AddText(const rstl::wstring& text) {
#if RSTL_CORRUPTION
  if (mTextBufferState->mRenderBuffer) {
#else
  if (mRenderBuffer) {
#endif
    mPrimitiveStartTimes.reserve(mPrimitiveStartTimes.size() + 1);
    mPrimitiveStartTimes.push_back_unsafe(
        rstl::pair< float, int >(rstl::max_val(GetCurrentAnimationOverAge(), mCurrentTime),
#if RSTL_CORRUPTION
                                 mTextBufferState->mRenderBuffer->GetNumPrimitives()));
#else
                                 mRenderBuffer->GetNumPrimitives()));
#endif
  }
  mText.append(text);
  ClearRenderBuffer();
}

void CGuiTextSupport::SetWordWrap(bool wordWrap) {
  if (wordWrap != mProperties.mWordWrap) {
    mProperties.mWordWrap = wordWrap;
    ClearRenderBuffer();
  }
}

void CGuiTextSupport::SetImageBaseline(bool baseline) {
  if (mImageBaseline != baseline) {
    mImageBaseline = baseline;
#if !RSTL_CORRUPTION
    ClearRenderBuffer();
  }
}

void CGuiTextSupport::SetJustification(EJustification justification) {
  if (justification != mProperties.mJustification) {
    mProperties.mJustification = justification;
#endif
    ClearRenderBuffer();
  }
}

void CGuiTextSupport::SetVerticalJustification(EVerticalJustification justification) {
  if (justification != mProperties.mVerticalJustification) {
    mProperties.mVerticalJustification = justification;
    ClearRenderBuffer();
  }
}

void CGuiTextSupport::SetFontColor(const CColor& color) {
  if (!(mFontColor == color)) {
    ClearRenderBuffer();
    mFontColor = color;
  }
}

void CGuiTextSupport::SetOutlineColor(const CColor& color) {
  if (!(mOutlineColor == color)) {
    ClearRenderBuffer();
    mOutlineColor = color;
  }
}

void CGuiTextSupport::SetGeometryColor(const CColor& color) { mGeometryColor = color; }

void CGuiTextSupport::SetControlTXTRMap(
    const rstl::vector< rstl::pair< CAssetId, CAssetId > >* textureMap) {
  if (mProperties.mTextureMap != textureMap) {
    mProperties.mTextureMap = textureMap;
    ClearRenderBuffer();
  }
}

void CGuiTextSupport::Render() const {
  CheckAndRebuildRenderBuffer();
  const CTransform4f oldModel = CGraphics::GetModelMatrix();
  CGraphics::SetModelMatrix(oldModel * CTransform4f::Scale(CVector3f(1.f, 1.f, -1.f)));
  if (const CTextRenderBuffer* buffer = GetCurrentPageRenderBuffer()) {
    buffer->Render(mGeometryColor, mCurrentTimeMod900);
  }
  CGraphics::SetModelMatrix(oldModel);
}

void CGuiTextSupport::CheckAndRebuildTextBuffer() const {
#if RSTL_CORRUPTION
  CTextExecuteBuffer& buffer = mTextBufferState->mExecuteBuffer;
  buffer.Clear();
  buffer.AddWordWrapping(mProperties.mWordWrap);
  buffer.BeginBlock(0, 0, mExtentX, mExtentY, mImageBaseline, kTD_Horizontal,
#else
  mExecuteBuffer.Clear();
  mExecuteBuffer.AddWordWrapping(mProperties.mWordWrap);
  mExecuteBuffer.BeginBlock(0, 0, mExtentX, mExtentY, mImageBaseline, kTD_Horizontal,
#endif
                            mProperties.mJustification, mProperties.mVerticalJustification);
#if RSTL_CORRUPTION
  buffer.AddColor(kCT_Main, mFontColor.GetRed(), mFontColor.GetGreen(), mFontColor.GetBlue(),
                  mFontColor.GetAlpha());
  buffer.AddColor(kCT_Outline, mOutlineColor.GetRed(), mOutlineColor.GetGreen(),
#else
  mExecuteBuffer.AddColor(kCT_Main, mFontColor.GetRed(), mFontColor.GetGreen(),
                          mFontColor.GetBlue(), mFontColor.GetAlpha());
  mExecuteBuffer.AddColor(kCT_Outline, mOutlineColor.GetRed(), mOutlineColor.GetGreen(),
#endif
                          mOutlineColor.GetBlue(), mOutlineColor.GetAlpha());
#if RSTL_CORRUPTION
  buffer.AddCharacterExtraSpace(mExtraCharacterSpacing);
  buffer.AddLineExtraSpace(mExtraLineSpacing);
  if (!close_enough(mLineSpacingScale, 0.f)) {
    buffer.AddLineSpacing(mLineSpacingScale);
  }
#else
  mExecuteBuffer.AddCharacterExtraSpace(mExtraCharacterSpacing);
  mExecuteBuffer.AddLineExtraSpace(mExtraLineSpacing);
#if RSTL_GUI_TEXT_LINE_SPACING
  if (!close_enough(mLineSpacing, 0.f)) {
    mExecuteBuffer.AddLineSpacing(mLineSpacing);
  }
#endif
#endif
  rstl::wstring text;
  if (mFontId != kInvalidAssetId) {
#if RSTL_CORRUPTION
    text = rstl::wstring(CStringExtras::ConvertToUNICODE(
        CStringExtras::Format("&font=%s;", mFontId.ToHexString().c_str())));
#else
    text = rstl::wstring(
        CStringExtras::ConvertToUNICODE(rstl::string(CBasics::Stringize("&font=%8.8X;", mFontId))));
#endif
  }
  text.append(mText);
  CTextParser parser(*mPool);
#if RSTL_CORRUPTION
  parser.ParseText(buffer, text.c_str(), text.size(), mProperties.mTextureMap);
  buffer.EndBlock();
#else
  parser.ParseText(mExecuteBuffer, text.c_str(), text.size(), mProperties.mTextureMap);
  mExecuteBuffer.EndBlock();
#endif
}

bool CGuiTextSupport::CheckAndRebuildRenderBuffer() const {
#if RSTL_CORRUPTION
  if ((!mMultipage && !mTextBufferState->mRenderBuffer) ||
      (mMultipage && mTextBufferState->mPages.empty())) {
#else
  if ((!mMultipage && !mRenderBuffer) || (mMultipage && mPages.empty())) {
#endif
    CheckAndRebuildTextBuffer();
#if RSTL_CORRUPTION
    mTextBufferState->mAssets = mTextBufferState->mExecuteBuffer.GetAssets();
#else
    mAssets = mExecuteBuffer.GetAssets();
#endif
    if (_GetIsTextSupportFinishedLoading()) {
      CheckAndRebuildTextBuffer();
      if (mMultipage) {
#if RSTL_CORRUPTION
        mTextBufferState->mPages =
            mTextBufferState->mExecuteBuffer.BuildRenderBufferPages(CVector2i(mExtentX, mExtentY));
#else
        mPages = mExecuteBuffer.BuildRenderBufferPages(CVector2i(mExtentX, mExtentY));
#endif
      } else {
#if RSTL_CORRUPTION
        mTextBufferState->mRenderBuffer = mTextBufferState->mExecuteBuffer.BuildRenderBuffer();
        mTextBufferState->mBounds = mTextBufferState->mRenderBuffer->GetTextBounds();
#else
        mRenderBuffer = mExecuteBuffer.BuildRenderBuffer();
        mBounds = mRenderBuffer->GetTextBounds();
#endif
      }
#if RSTL_CORRUPTION
      mTextBufferState->mExecuteBuffer.Clear();
#else
      mExecuteBuffer.Clear();
#endif
    } else {
      return false;
    }
    const_cast< CGuiTextSupport* >(this)->Update(0.f);
  }
  return true;
}

void CGuiTextSupport::ClearRenderBuffer() {
#if RSTL_CORRUPTION
  mTextBufferState->mRenderBuffer.clear();
  mTextBufferState->mPages = rstl::list< CTextRenderBuffer >();
#else
  mRenderBuffer.clear();
  mPages = rstl::list< CTextRenderBuffer >();
#endif
}

void CGuiTextSupport::Update(float dt) {
  if (mTypewriterEnabled) {
    CTextRenderBuffer* buffer = GetCurrentPageRenderBuffer();
    if (buffer != nullptr) {
      float characterStartTime = 0.f;
      for (int i = 0; i < buffer->GetNumPrimitives(); ++i) {
        for (int j = 0; j < mPrimitiveStartTimes.size(); ++j) {
          const rstl::pair< float, int >& start = mPrimitiveStartTimes[j];
          if (start.second < i) {
            continue;
          }
          if (start.second != i) {
            break;
          }
          characterStartTime = start.first;
          break;
        }
        CTextRenderBuffer::Primitive primitive = buffer->GetPrimitive(i);
        float alpha = rstl::min_val(
            1.f, rstl::max_val(0.f, (mCurrentTime - characterStartTime) / mCharacterFadeTime));
        characterStartTime += 1.f / mCharacterRate;
        CColor color(primitive.mColor);
        color.SetAlpha(alpha);
        primitive.mColor = color.GetColor_u32();
        buffer->SetPrimitive(primitive, i);
      }
    }
    mCurrentTime += dt;
  }
  mCurrentTimeMod900 = fmod(mCurrentTimeMod900 + dt, 900.0);
}

void CGuiTextSupport::SetTypeWriteEffectOptions(bool enable, float fadeTime, float rate) {
  mTypewriterEnabled = enable;
  mCharacterFadeTime = rstl::max_val(fadeTime, 0.0001f);
  mCharacterRate = rstl::max_val(rate, 1.f);
}

float CGuiTextSupport::GetTotalAnimationTime() const {
  if (const CTextRenderBuffer* buffer = GetCurrentPageRenderBuffer()) {
    if (mTypewriterEnabled) {
      return buffer->GetNumPrimitives() / mCharacterRate;
    }
  }
  return 0.f;
}

float CGuiTextSupport::GetNumCharactersPrinted() const {
  if (const CTextRenderBuffer* buffer = GetCurrentPageRenderBuffer()) {
    if (mTypewriterEnabled) {
      return rstl::min_val(float(buffer->GetNumPrimitives()), mCurrentTime * mCharacterRate);
    }
  }
  return 0.f;
}

#if !RSTL_CORRUPTION
float CGuiTextSupport::GetNumCharsTotal() const {
  if (const CTextRenderBuffer* buffer = GetCurrentPageRenderBuffer()) {
    if (mTypewriterEnabled) {
      return buffer->GetNumPrimitives();
    }
  }
  return 0.f;
}

#endif
float CGuiTextSupport::GetCurrentAnimationOverAge() const {
  float time = 0.f;
  if (const CTextRenderBuffer* buffer = GetCurrentPageRenderBuffer()) {
    if (mTypewriterEnabled) {
      if (!mPrimitiveStartTimes.empty()) {
        const rstl::pair< float, int >& last = mPrimitiveStartTimes.back();
        time = rstl::max_val(time, (buffer->GetNumPrimitives() - last.second) / mCharacterRate +
                                       last.first);
      } else {
        time = rstl::max_val(time, buffer->GetNumPrimitives() / mCharacterRate);
      }
    }
  }
  return time;
}

int CGuiTextSupport::GetTotalPageCount() {
  if (CheckAndRebuildRenderBuffer()) {
#if RSTL_CORRUPTION
    return mTextBufferState->mPages.size();
#else
    return mPages.size();
#endif
  }
  return -1;
}

void CGuiTextSupport::SetPage(int page) {
  mPageCounter = page;
  mPrimitiveStartTimes.clear();
  mCurrentTime = 0.f;
}

CTextRenderBuffer* CGuiTextSupport::GetCurrentPageRenderBuffer() const {
#if RSTL_CORRUPTION
  STextBufferState& state = *mTextBufferState.get();
  if (state.mRenderBuffer && !mMultipage) {
    return state.mRenderBuffer.get_ptr();
#else
  if (mRenderBuffer && !mMultipage) {
    return mRenderBuffer.get_ptr();
#endif
  }
#if RSTL_CORRUPTION
  if (mMultipage && state.mPages.size() > mPageCounter) {
#else
  if (mMultipage && mPages.size() > mPageCounter) {
#endif
    int i = 0;
#if RSTL_CORRUPTION
    for (rstl::list< CTextRenderBuffer >::iterator it = state.mPages.begin();; ++it, ++i) {
#else
    for (rstl::list< CTextRenderBuffer >::iterator it = mPages.begin();; ++it, ++i) {
#endif
      if (i == mPageCounter) {
        return &*it;
      }
    }
  }
  return nullptr;
}

const rstl::pair< CVector2i, CVector2i >& CGuiTextSupport::GetBounds() {
  CheckAndRebuildRenderBuffer();
#if RSTL_CORRUPTION
  return mTextBufferState->mBounds;
#else
  return mBounds;
#endif
}

void ScreenText::DrawExecuteBuffer(const CTextExecuteBuffer& buffer) {
#if RSTL_CORRUPTION
  CViewport viewport = CGraphics::GetViewport();
  const float top = CCast::LtoF(viewport.mTop);
  const float bottom = CCast::LtoF(viewport.mTop + viewport.mHeight);
#else
  const CViewport viewport = CGraphics::GetViewport();
#endif
  CGraphics::SetViewPointMatrix(CTransform4f::Identity());
#if RSTL_CORRUPTION
  const int& left = viewport.mLeft;
  const int right = viewport.mLeft + viewport.mWidth;
  CGraphics::SetOrtho(left, right, bottom, top, -4096.f, 4096.f);
#else
  CGraphics::SetOrtho(static_cast< float >(viewport.mLeft),
                      static_cast< float >(viewport.mLeft + viewport.mWidth),
                      static_cast< float >(viewport.mTop + viewport.mHeight),
                      static_cast< float >(viewport.mTop), -4096.f, 4096.f);
#endif
  CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
  CGraphics::SetCullMode(kCM_None);
  CGraphics::SetDepthWriteMode(true, kE_Always, false);
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);

  const CTransform4f transform =
      CTransform4f::FromColumns(CVector3f::Right(), CVector3f::Forward(), CVector3f::Down(),
                                CVector3f(0.f, 0.f, static_cast< float >(viewport.mHeight)));
  CGraphics::SetModelMatrix(transform);

  buffer.BuildRenderBuffer().Render(CColor::White(), 0.f);
  CGraphics::SetCullMode(kCM_Front);
}

void ScreenText::DrawString(const rstl::string& text, int x, int y,
                            const TToken< CRasterFont >& font) {
  const CViewport viewport = CGraphics::GetViewport();
  CTextExecuteBuffer buffer;
  buffer.AddWordWrapping(true);
  buffer.BeginBlock(x, y, viewport.mWidth - x, viewport.mHeight - y, false, kTD_Horizontal,
                    kJustification_Left, kVerticalJustification_Center);

  {
    const TToken< CRasterFont > fontCopy = font;
    buffer.AddFont(fontCopy);
  }
  buffer.AddString(CStringExtras::ConvertToUNICODE(text));
  buffer.EndBlock();
  DrawExecuteBuffer(buffer);
}
#if !RSTL_CORRUPTION

void ScreenText::DrawString(const rstl::string& text, int x, int y,
                            const TToken< CRasterFont >& font, const CColor& color, bool shadow) {
  DrawString(CStringExtras::ConvertToUNICODE(text), x, y, font, color, shadow);
}

static inline void DrawStringPass(const rstl::wstring& text, int x, int y, const CViewport& viewport,
                                  bool shadow, const TToken< CRasterFont >& font, const CColor& color) {
  CTextExecuteBuffer buffer;
  buffer.AddWordWrapping(true);
  const int offset = shadow ? 1 : 0;
  buffer.BeginBlock(x + offset, y + offset, viewport.mWidth - x, viewport.mHeight - y,
                    false, kTD_Horizontal, kJustification_Left, kVerticalJustification_Center);
  if (!shadow) {
    buffer.AddColor(kCT_Main, CTextColor(color.GetRedu8(), color.GetGreenu8(), color.GetBlueu8(),
                                         color.GetAlphau8()));
  } else {
    buffer.AddColor(kCT_Main, CTextColor(0, 0, 0, 255));
  }
  {
    const TToken< CRasterFont > fontCopy = font;
    buffer.AddFont(fontCopy);
  }
  buffer.AddString(text);
  buffer.EndBlock();
  ScreenText::DrawExecuteBuffer(buffer);
}

void ScreenText::DrawString(const rstl::wstring& text, int x, int y,
                            const TToken< CRasterFont >& font, const CColor& color, bool shadow) {
  const CViewport viewport = CGraphics::GetViewport();
  if (shadow) {
    DrawStringPass(text, x, y, viewport, true, font, color);
  }
  DrawStringPass(text, x, y, viewport, false, font, color);
}
#endif
