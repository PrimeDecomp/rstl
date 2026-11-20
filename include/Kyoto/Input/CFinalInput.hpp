#ifndef _CFINALINPUT
#define _CFINALINPUT
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"

#include "rstl/types.h"

#include "Kyoto/Input/CControllerData.hpp"
#include "Kyoto/Input/CControllerGamepadData.hpp"

class COsContext;
class CVector2f;

class CFinalInputInfo;
class CControllerData;

#if RSTL_FINAL_INPUT_ANALOG_THRESHOLDS && !RSTL_FINAL_INPUT_PAD_SNAPSHOT && defined(__MWERKS__)
#pragma cpp_extensions on
#endif

class CFinalInput {
  // Guessed names for Wii device queries, corroborated by GUI direction/button consumers.
  enum EWiiInput {
    kWI_A = 3,
    kWI_B = 4,
    kWI_DPadUp = 8,
    kWI_DPadDown = 9,
    kWI_DPadRight = 10,
    kWI_DPadLeft = 11,
    kWI_LeftStickUp = 13,
    kWI_LeftStickDown = 16,
    kWI_LeftStickRight = 19,
    kWI_LeftStickLeft = 20
  };

public:
#if RSTL_FINAL_INPUT_ANALOG_THRESHOLDS
  static const float kInput_AnalogOnThreshhold;
  static const float kInput_AnalogTriggerOnThreshhold;
#endif
  static const float kAnalogAxisDigitalThreshold;
  static const float kRollDigitalThreshold;
  static const float kPitchDigitalThreshold;
  static const float kPointerDigitalThreshold;

  bool GetKnownDeviceIsPresent() const;
  bool CheckValidControl(int control) const;
  float GetAnalog(int control) const;
  float GetMotionAnalog(int control) const;
  float GetSwingAnalog(int control) const;
  bool GetDigital(int control) const;
  bool GetMotionDigital(int control) const;
  bool GetSwingDigital(int control) const;
  bool GetPressed(int control) const;
  bool GetMotionPressed(int control) const;
  bool GetSwingPressed(int control) const;
  bool GetReleased(int control) const;
  bool GetMotionReleased(int control) const;
  bool GetSwingReleased(int control) const;
  CVector2f GetPointerPosition() const;

  // Control names are inferred from the Trilogy mapping and motion-mask ordering.
  enum EPhysicalControl {
    kPC_None = 0,
    kPC_Home = 1,
    kPC_Minus = 2,
    kPC_A = 3,
    kPC_B = 4,
    kPC_Two = 5,
    kPC_One = 6,
    kPC_Plus = 7,
    kPC_DPadUp = 8,
    kPC_DPadDown = 9,
    kPC_DPadRight = 10,
    kPC_DPadLeft = 11,
    kPC_AnyDPad = 12,
    kPC_StickUp = 13,
    kPC_StickDiagonal2Negative = 14,
    kPC_StickDiagonal3Positive = 15,
    kPC_StickDown = 16,
    kPC_StickDiagonal3Negative = 17,
    kPC_StickDiagonal2Positive = 18,
    kPC_StickRight = 19,
    kPC_StickLeft = 20,
    kPC_NunchukZ = 21,
    kPC_NunchukC = 22,
    kPC_NunchukRollNegative = 23,
    kPC_NunchukRollPositive = 24,
    kPC_NunchukPitchNegative = 25,
    kPC_NunchukPitchPositive = 26,
    kPC_PointerUp = 27,
    kPC_PointerDown = 28,
    kPC_PointerRight = 29,
    kPC_PointerLeft = 30,
    kPC_WiimoteRollNegative = 31,
    kPC_WiimoteRollPositive = 32,
    kPC_WiimotePitchNegative = 33,
    kPC_WiimotePitchPositive = 34,
  };

  enum EMotionControl {
    kMC_WiimoteImpulseX = 0,
    kMC_WiimoteShakeX = 1,
    kMC_WiimoteShakeZ = 2,
    kMC_WiimoteGesture1 = 3,
    kMC_WiimoteGesture0 = 4,
    kMC_WiimoteGesture2 = 5,
    kMC_WiimoteGesture3 = 6,
    kMC_WiimoteMotionIntegral = 7,
    kMC_NunchukImpulseX = 8,
    kMC_NunchukShakeX = 9,
    kMC_NunchukShakeZ = 10,
    kMC_NunchukGesture1 = 11,
    kMC_NunchukGesture0 = 12,
    kMC_NunchukGesture2 = 13,
    kMC_NunchukGesture3 = 14,
    kMC_NunchukMotionIntegral = 15,
  };

  enum ESwingControl {
    kSC_WiimoteYMinBeforeMax = 0,
    kSC_WiimoteYMaxBeforeMin = 1,
    kSC_WiimoteXMinBeforeMax = 2,
    kSC_WiimoteXMaxBeforeMin = 3,
    kSC_WiimoteZMaxBeforeMin = 4,
    kSC_WiimoteZMinBeforeMax = 5,
    kSC_NunchukYMinBeforeMax = 6,
    kSC_NunchukYMaxBeforeMin = 7,
    kSC_NunchukXMinBeforeMax = 8,
    kSC_NunchukXMaxBeforeMin = 9,
    kSC_NunchukZMaxBeforeMin = 10,
    kSC_NunchukZMinBeforeMax = 11,
  };

  CFinalInput();
#if RSTL_FINAL_INPUT_PAD_SNAPSHOT
  CFinalInput(int controllerIdx, float dt, const CControllerGamepadData& data);
#else
  CFinalInput(const CFinalInputInfo& info, float leftDiv, float rightDiv);
  // Guessed Wii snapshot type name and input-type meaning.
  CFinalInput(int channel, float dt, const CControllerData& data, uint inputType);
  CFinalInput(int channel, float dt, const CControllerGamepadData& data, float leftDiv,
              float rightDiv);
  CFinalInput(int channel, float dt, const COsContext& context);
#endif
#if RSTL_FINAL_INPUT_ANALOG_THRESHOLDS && !RSTL_FINAL_INPUT_PAD_SNAPSHOT
  CFinalInput(const CFinalInput& other)
  : mDt(other.mDt)
  , mControllerIdx(other.mControllerIdx)
#if RSTL_FINAL_INPUT_CONTROLLER_DATA
  , mInputType(other.mInputType)
  , mControllerData(other.mControllerData)
#else
  , mAnaLeftX(other.mAnaLeftX)
  , mAnaLeftY(other.mAnaLeftY)
  , mAnaRightX(other.mAnaRightX)
  , mAnaRightY(other.mAnaRightY)
  , mAnaLeftTrigger(other.mAnaLeftTrigger)
  , mAnaRightTrigger(other.mAnaRightTrigger)
#if !RSTL_FINAL_INPUT_PACKED_FLAGS
  , mEnableAnaLeftXP(other.mEnableAnaLeftXP)
  , mEnableAnaLeftYP(other.mEnableAnaLeftYP)
  , mEnableAnaRightXP(other.mEnableAnaRightXP)
  , mEnableAnaRightYP(other.mEnableAnaRightYP)
#endif
  , mAnaLeftTriggerP(other.mAnaLeftTriggerP)
  , mAnaRightTriggerP(other.mAnaRightTriggerP)
  , btns1(other.btns1)
  , btns2(other.btns2)
  , btns3(other.btns3)
#if RSTL_FINAL_INPUT_PACKED_FLAGS
  , btns4(other.btns4)
#endif
#endif
  {
  }
#endif

#if !RSTL_FINAL_INPUT_PAD_SNAPSHOT
  void InitializeAnalog(float leftDiv, float rightDiv);
  CFinalInput ScaleAnalogueSticks(float leftDiv, float rightDiv) const;
  static CFinalInput NoInput(int channel, float dt);
#endif

  float Time() const { return mDt; }
  void SetTime(float dt) { mDt = dt; }
  float DeltaTime() const { return mDt; }
  void SetDeltaTime(float dt) { mDt = dt; }
#if !RSTL_FINAL_INPUT_PAD_SNAPSHOT
  float GetAnalogLeftX() const;
  float GetAnalogLeftY() const;
  float GetAnalogRightX() const;
  float GetAnalogRightY() const;
#endif

  int ControllerNumber() const { return mControllerIdx; }

#if RSTL_FINAL_INPUT_CONTROLLER_DATA
  uint GetInputType() const { return mInputType; }
  const CControllerData& GetControllerData() const { return mControllerData; }
#endif

#if !RSTL_FINAL_INPUT_PAD_SNAPSHOT
  // Legacy GameCube-style queries: analog strength (A*), held (D*) and pressed this frame (P*).
  float ALAUp() const;
  float ALADown() const;
  float ALALeft() const;
  float ALARight() const;
  float ARAUp() const;
  float ARADown() const;
  float ARALeft() const;
  float ARARight() const;
  float ALTrigger() const;
  float ARTrigger() const;
  float ADPUp() const;
  float ADPDown() const;
  float ADPLeft() const;
  float ADPRight() const;
  float AA() const;
  float AB() const;
  float AX() const;
  float AY() const;
  float AZ() const;
  float AL() const;
  float AR() const;
  float AStart() const;
  float ALeftX() const;
  float ALeftY() const;
  bool DLAUp() const;
  bool DLADown() const;
  bool DLALeft() const;
  bool DLARight() const;
  bool DRAUp() const;
  bool DRADown() const;
  bool DRALeft() const;
  bool DRARight() const;
  bool DLTrigger() const;
  bool DRTrigger() const;
  bool DDPUp() const;
  bool DDPDown() const;
  bool DDPLeft() const;
  bool DDPRight() const;
  bool DA() const;
  bool DB() const;
  bool DX() const;
  bool DY() const;
  bool DZ() const;
  bool DL() const;
  bool DR() const;
  bool DStart() const;
  bool PLAUp() const;
  bool PLADown() const;
  bool PLALeft() const;
  bool PLARight() const;
  bool PRAUp() const;
  bool PRADown() const;
  bool PRALeft() const;
  bool PRARight() const;
  bool PLTrigger() const;
  bool PRTrigger() const;
  bool PDPUp() const;
  bool PDPDown() const;
  bool PDPLeft() const;
  bool PDPRight() const;
  bool PA() const;
  bool PB() const;
  bool PX() const;
  bool PY() const;
  bool PZ() const;
  bool PL() const;
  bool PR() const;
  bool PStart() const;
#endif

private:
  friend class CFinalInputInfo;

  float mDt;
  int mControllerIdx;
#if RSTL_FINAL_INPUT_CONTROLLER_DATA
  // The native Wii record is a header followed by the complete device snapshot.
  uint mInputType; // Inferred meaning; native word at offset8.
  CControllerData mControllerData;
#elif RSTL_FINAL_INPUT_PAD_SNAPSHOT
  CControllerGamepadData mPad; // Guessed name
#else
  float mAnaLeftX;
  float mAnaLeftY;
  float mAnaRightX;
  float mAnaRightY;
  float mAnaLeftTrigger;
  float mAnaRightTrigger;
#if !RSTL_FINAL_INPUT_PACKED_FLAGS
  bool mEnableAnaLeftXP;
  bool mEnableAnaLeftYP;
  bool mEnableAnaRightXP;
  bool mEnableAnaRightYP;
#endif
  float mAnaLeftTriggerP;
  float mAnaRightTriggerP;

  // Both games hold the same fields. Prime keeps the analog press flags as separate bytes ahead
  // of the bitfields, while Echoes packs them into the first button byte (and spills into a
  // fourth); only the bit positions differ, so the names carry none.
#if RSTL_FINAL_INPUT_PACKED_FLAGS
  union {
    struct {
      uchar mEnableAnaLeftXP : 1;
      uchar mEnableAnaLeftYP : 1;
      uchar mEnableAnaRightXP : 1;
      uchar mEnableAnaRightYP : 1;
      uchar mA : 1;
      uchar mB : 1;
      uchar mX : 1;
      uchar mY : 1;
    };
    uchar btns1;
  };
  union {
    struct {
      uchar mZ : 1;
      uchar mL : 1;
      uchar mR : 1;
      uchar mDPUp : 1;
      uchar mDPRight : 1;
      uchar mDPDown : 1;
      uchar mDPLeft : 1;
      uchar mStart : 1;
    };
    uchar btns2;
  };
  union {
    struct {
      uchar mPA : 1;
      uchar mPB : 1;
      uchar mPX : 1;
      uchar mPY : 1;
      uchar mPZ : 1;
      uchar mPL : 1;
      uchar mPR : 1;
      uchar mPDPUp : 1;
    };
    uchar btns3;
  };
  union {
    struct {
      uchar mPDPRight : 1;
      uchar mPDPDown : 1;
      uchar mPDPLeft : 1;
      uchar mPStart : 1;
    };
    uchar btns4;
  };
#else
  union {
    struct {
      uchar mA : 1;
      uchar mB : 1;
      uchar mX : 1;
      uchar mY : 1;
      uchar mZ : 1;
      uchar mL : 1;
      uchar mR : 1;
      uchar mDPUp : 1;
    };
    uchar btns1;
  };
  union {
    struct {
      uchar mDPRight : 1;
      uchar mDPDown : 1;
      uchar mDPLeft : 1;
      uchar mStart : 1;
      uchar mPA : 1;
      uchar mPB : 1;
      uchar mPX : 1;
      uchar mPY : 1;
    };
    uchar btns2;
  };
  union {
    struct {
      uchar mPZ : 1;
      uchar mPL : 1;
      uchar mPR : 1;
      uchar mPDPUp : 1;
      uchar mPDPRight : 1;
      uchar mPDPDown : 1;
      uchar mPDPLeft : 1;
      uchar mPStart : 1;
    };
    uchar btns3;
  };
#endif
#endif
};
#if RSTL_FINAL_INPUT_ANALOG_THRESHOLDS && !RSTL_FINAL_INPUT_PAD_SNAPSHOT && defined(__MWERKS__)
#pragma cpp_extensions reset
#endif

DECLARE_FULL_SIZE_FOR(CFinalInput,
                      RSTL_FINAL_INPUT_CONTROLLER_DATA
                          ? 0x218
                          : (RSTL_FINAL_INPUT_PAD_SNAPSHOT ? 0x108 : (RSTL_PRIME1 ? 0x30 : 0x2c)))
CHECK_SIZEOF(CFinalInput, CFinalInput_FULL_SIZE)

#if RSTL_FINAL_INPUT_CONTROLLER_DATA && !RSTL_PRIME1
// Recovered Wii bindings use the native input-query interface. Other legacy
// entry points remain declared while their Wii callers are being recovered.
inline bool CFinalInput::DLAUp() const {
    return GetDigital(kWI_LeftStickUp);
  }

inline bool CFinalInput::DLADown() const {
    return GetDigital(kWI_LeftStickDown);
  }

inline bool CFinalInput::DLALeft() const {
    return GetDigital(kWI_LeftStickLeft);
  }

inline bool CFinalInput::DLARight() const {
    return GetDigital(kWI_LeftStickRight);
  }

inline bool CFinalInput::DDPUp() const {
    return GetDigital(kWI_DPadUp);
  }

inline bool CFinalInput::DDPDown() const {
    return GetDigital(kWI_DPadDown);
  }

inline bool CFinalInput::DDPLeft() const {
    return GetDigital(kWI_DPadLeft);
  }

inline bool CFinalInput::DDPRight() const {
    return GetDigital(kWI_DPadRight);
  }

inline bool CFinalInput::PDPLeft() const {
    return GetPressed(kWI_DPadLeft);
  }

inline bool CFinalInput::PDPRight() const {
    return GetPressed(kWI_DPadRight);
  }

inline bool CFinalInput::PA() const {
    return GetPressed(kWI_A);
  }

inline bool CFinalInput::PB() const {
    return GetPressed(kWI_B);
  }
#endif

#if !RSTL_FINAL_INPUT_CONTROLLER_DATA && !RSTL_FINAL_INPUT_PAD_SNAPSHOT
inline float CFinalInput::GetAnalogLeftX() const { return mAnaLeftX; }

inline float CFinalInput::GetAnalogLeftY() const { return mAnaLeftY; }

inline float CFinalInput::GetAnalogRightX() const { return mAnaRightX; }

inline float CFinalInput::GetAnalogRightY() const { return mAnaRightY; }

inline float CFinalInput::ALAUp() const { return mAnaLeftY > 0.f ? mAnaLeftY : 0.f; }

inline float CFinalInput::ALADown() const { return mAnaLeftY < 0.f ? -mAnaLeftY : 0.f; }

inline float CFinalInput::ALALeft() const { return mAnaLeftX < 0.f ? -mAnaLeftX : 0.f; }

inline float CFinalInput::ALARight() const { return mAnaLeftX > 0.f ? mAnaLeftX : 0.f; }

inline float CFinalInput::ARAUp() const { return mAnaRightY > 0.f ? mAnaRightY : 0.f; }

inline float CFinalInput::ARADown() const { return mAnaRightY < 0.f ? -mAnaRightY : 0.f; }

inline float CFinalInput::ARALeft() const { return mAnaRightX < 0.f ? -mAnaRightX : 0.f; }

inline float CFinalInput::ARARight() const { return mAnaRightX > 0.f ? mAnaRightX : 0.f; }

inline float CFinalInput::ALTrigger() const { return mAnaLeftTrigger; }

inline float CFinalInput::ARTrigger() const { return mAnaRightTrigger; }

inline float CFinalInput::ADPUp() const { return mDPUp ? 1.f : 0.f; }

inline float CFinalInput::ADPDown() const { return mDPDown ? 1.f : 0.f; }

inline float CFinalInput::ADPLeft() const { return mDPLeft ? 1.f : 0.f; }

inline float CFinalInput::ADPRight() const { return mDPRight ? 1.f : 0.f; }

inline float CFinalInput::AA() const { return mA ? 1.f : 0.f; }

inline float CFinalInput::AB() const { return mB ? 1.f : 0.f; }

inline float CFinalInput::AX() const { return mX ? 1.f : 0.f; }

inline float CFinalInput::AY() const { return mY ? 1.f : 0.f; }

inline float CFinalInput::AZ() const { return mZ ? 1.f : 0.f; }

inline float CFinalInput::AL() const { return mL ? 1.f : 0.f; }

inline float CFinalInput::AR() const { return mR ? 1.f : 0.f; }

inline float CFinalInput::AStart() const { return mStart ? 1.f : 0.f; }

inline bool CFinalInput::DLAUp() const {
    return kInput_AnalogOnThreshhold < mAnaLeftY ? true : false;
  }

inline bool CFinalInput::DLADown() const {
    return -kInput_AnalogOnThreshhold > mAnaLeftY ? true : false;
  }

inline bool CFinalInput::DLALeft() const {
    return -kInput_AnalogOnThreshhold > mAnaLeftX ? true : false;
  }

inline bool CFinalInput::DLARight() const {
    return kInput_AnalogOnThreshhold < mAnaLeftX ? true : false;
  }

inline bool CFinalInput::DRAUp() const { return kInput_AnalogOnThreshhold < mAnaRightY ? true : false; }

inline bool CFinalInput::DRADown() const { return -kInput_AnalogOnThreshhold > mAnaRightY ? true : false; }

inline bool CFinalInput::DRALeft() const { return -kInput_AnalogOnThreshhold > mAnaRightX ? true : false; }

inline bool CFinalInput::DRARight() const { return kInput_AnalogOnThreshhold < mAnaRightX ? true : false; }

inline bool CFinalInput::DLTrigger() const {
    return kInput_AnalogTriggerOnThreshhold < mAnaLeftTrigger ? true : false;
  }

inline bool CFinalInput::DRTrigger() const {
    return kInput_AnalogTriggerOnThreshhold < mAnaRightTrigger ? true : false;
  }

inline bool CFinalInput::DDPUp() const {
    return mDPUp;
  }

inline bool CFinalInput::DDPDown() const {
    return mDPDown;
  }

inline bool CFinalInput::DDPLeft() const {
    return mDPLeft;
  }

inline bool CFinalInput::DDPRight() const {
    return mDPRight;
  }

inline bool CFinalInput::DA() const { return mA; }

inline bool CFinalInput::DB() const { return mB; }

inline bool CFinalInput::DX() const { return mX; }

inline bool CFinalInput::DY() const { return mY; }

inline bool CFinalInput::DZ() const { return mZ; }

inline bool CFinalInput::DL() const { return mL; }

inline bool CFinalInput::DR() const { return mR; }

inline bool CFinalInput::DStart() const { return mStart; }

inline bool CFinalInput::PLAUp() const {
    return DLAUp() && mEnableAnaLeftYP ? true : false;
  }

inline bool CFinalInput::PLADown() const {
    return -kInput_AnalogOnThreshhold > mAnaLeftY && mEnableAnaLeftYP ? true : false;
  }

inline bool CFinalInput::PLALeft() const {
    return -kInput_AnalogOnThreshhold > mAnaLeftX && mEnableAnaLeftXP ? true : false;
  }

inline bool CFinalInput::PLARight() const {
    return kInput_AnalogOnThreshhold < mAnaLeftX && mEnableAnaLeftXP ? true : false;
  }

inline bool CFinalInput::PRAUp() const {
    return kInput_AnalogOnThreshhold < mAnaRightY && mEnableAnaRightYP ? true : false;
  }

inline bool CFinalInput::PRADown() const {
    return -kInput_AnalogOnThreshhold > mAnaRightY && mEnableAnaRightYP ? true : false;
  }

inline bool CFinalInput::PRALeft() const {
    return -kInput_AnalogOnThreshhold > mAnaRightX && mEnableAnaRightXP ? true : false;
  }

inline bool CFinalInput::PRARight() const {
    return kInput_AnalogOnThreshhold < mAnaRightX && mEnableAnaRightXP ? true : false;
  }

inline bool CFinalInput::PLTrigger() const {
    return kInput_AnalogTriggerOnThreshhold < mAnaLeftTriggerP ? true : false;
  }

inline bool CFinalInput::PRTrigger() const {
    return kInput_AnalogTriggerOnThreshhold < mAnaRightTriggerP ? true : false;
  }

inline bool CFinalInput::PDPUp() const { return mPDPUp; }

inline bool CFinalInput::PDPDown() const { return mPDPDown; }

inline bool CFinalInput::PDPLeft() const {
    return mPDPLeft;
  }

inline bool CFinalInput::PDPRight() const {
    return mPDPRight;
  }

inline bool CFinalInput::PA() const {
    return mPA;
  }

inline bool CFinalInput::PB() const {
    return mPB;
  }

inline bool CFinalInput::PX() const { return mPX; }

inline bool CFinalInput::PY() const { return mPY; }

inline bool CFinalInput::PZ() const { return mPZ; }

inline bool CFinalInput::PL() const { return mPL; }

inline bool CFinalInput::PR() const { return mPR; }

inline bool CFinalInput::PStart() const { return mPStart; }

inline float CFinalInput::ALeftX() const { return mAnaLeftX; }

inline float CFinalInput::ALeftY() const { return mAnaLeftY; }
#endif

#endif // _CFINALINPUT
