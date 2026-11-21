#ifndef _CCOMPRESSEDANIMSOURCEREADER
#define _CCOMPRESSEDANIMSOURCEREADER

#include "rstl/static_assert.hpp"
#include "Kyoto/Animation/CAnimBitStream.hpp"
#include "Kyoto/Animation/CAnimChannelMask.hpp"
#include "Kyoto/Animation/CAnimKeyframeTimes.hpp"
#include "Kyoto/Animation/CAnimSourceReaderBase.hpp"
#include "Kyoto/Animation/CQuantizedAnimFrame.hpp"
#include "Kyoto/Animation/CVisibilityAnimationDataList.hpp"
#include "Kyoto/Animation/TSubAnimTypeToken.hpp"



class CCompressedAnimSource;
class CSegStatement;

class CCompressedAnimSourceReader : public CAnimSourceReaderBase {
public:
  CCompressedAnimSourceReader(const TSubAnimTypeToken< CCompressedAnimSource >& source,
                              CCharAnimTime time, const CAnimPOIData* poiData,
                              ushort currentKey = 0, float decodeTime = 0.f);

  // IAnimReader
  ~CCompressedAnimSourceReader() override;
  SAdvancementResults VAdvanceView(const CCharAnimTime& time) override;
  CCharAnimTime VGetTimeRemaining() const override;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const override;
  bool VHasOffset(const CSegId& seg) const override;
  CVector3f VGetOffset(const CSegId& seg) const override;
  CQuaternion VGetRotation(const CSegId& seg) const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& set) const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& set,
                           const CCharAnimTime& time) const override;
  void VGetJointData_Linear(const CCharLayoutInfo& layout, CJointData_LinearStorage& storage,
                            const CCharAnimTime& time) const override;
  void VGetJointData_Linear(const CCharLayoutInfo& layout,
                            CJointData_LinearStorage& storage) const override;
  const CVisibilityAnimationDataReader* VGetVisibilityDataAtCurrentTime() const override;
  const CVisibilityAnimationDataReader*
  VGetVisibilityDataAtTime(const CCharAnimTime& time) const override;
  rstl::ownership_transfer< IAnimReader > VClone() const override;
  void VSetPhase(float phase) override;
  SAdvancementResults VGetAdvancementResults(const CCharAnimTime& time,
                                             const CCharAnimTime& startOffset) const override;

  virtual bool VSupportsReverseView() const;                           // Guessed name
  virtual SAdvancementResults VReverseView(const CCharAnimTime& time); // Guessed name

private:
  // Guessed names
  void ResetDecodeState() const;
  void SeekDecodeTime(const CCharAnimTime& time) const;
  void GetSegStatement(CSegStatement& statement, const CSegId& seg) const;
  CQuaternion InterpolateQuantizedRotation(const short* first, const short* second, float t) const;
  CVector3f InterpolateQuantizedVector(const short* first, const short* second, float t,
                                       float scale) const;
  static void SetDecodeGQR6();

  float GetFrameInterpolantFromTime(float time) const;

  TSubAnimTypeToken< CCompressedAnimSource > mSource; // Guessed name
  mutable ushort mCurrentKey;                         // Guessed name
  CAnimKeyframeTimes mKeyframeTimes;                  // Guessed name
  CAnimChannelMask mChannelMask;                      // Guessed name
  mutable CAnimBitStream mBitStream;                  // Guessed name
  short* mFirstFrameBuffer;                           // Guessed name
  short* mSecondFrameBuffer;                          // Guessed name
  mutable CQuantizedAnimFrame mFirstFrame;            // Guessed name
  mutable CQuantizedAnimFrame mSecondFrame;           // Guessed name
  CSteadyStateAnimInfo mSteadyStateInfo;              // Guessed name
  mutable float mDecodeTime;                          // Guessed name
  CVisibilityAnimationDataList mVisibilityData;       // Guessed name
  bool mOwnsFrameBuffers;                             // Guessed name
};
CHECK_SIZEOF(CCompressedAnimSourceReader, 0xec)


#endif // _CCOMPRESSEDANIMSOURCEREADER
