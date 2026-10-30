#ifndef _CFINALINPUT
#define _CFINALINPUT
#include "rstl/RstlVersions.h"

#include "types.h"

#include "Kyoto/Input/CControllerData.hpp"

class COsContext;

#if RSTL_PRIME1
class CFinalInputInfo;
#endif

class CControllerGamepadData;
#if !RSTL_PRIME1
class CControllerData;
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION < RSTL_R3IJ && defined(__MWERKS__)
#pragma cpp_extensions on
#endif
#else
#ifdef __MWERKS__
#pragma cpp_extensions on
#endif
class CFinalInput {
  static const float kInput_AnalogOnThreshhold;
  static const float kInput_AnalogTriggerOnThreshhold;
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
class CFinalInput {
#else
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
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_R3IJ
public:
  static const float kAnalogAxisDigitalThreshold;
  static const float kRollDigitalThreshold;
  static const float kPitchDigitalThreshold;
  static const float kPointerDigitalThreshold;

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
  #else

  static const float kInput_AnalogOnThreshhold;
  static const float kInput_AnalogTriggerOnThreshhold;

public:
  #endif
#else
public:
  CFinalInput();
  // Guessed Wii snapshot type name and input-type meaning.
  CFinalInput(int channel, float dt, const CControllerData& data, uint inputType);
  CFinalInput(int channel, float dt, const CControllerGamepadData& data, float leftDif,
              float rightDiv);
  CFinalInput(int channel, float dt, const COsContext& context);
  CFinalInput(const CFinalInput& other)
  : mDt(other.mDt)
  , mControllerIdx(other.mControllerIdx)
#if RSTL_VERSION >= RSTL_R32J
  , mInputType(other.mInputType)
  , mControllerData(other.mControllerData)
#else
  , mAnaLeftX(other.mAnaLeftX)
  , mAnaLeftY(other.mAnaLeftY)
  , mAnaRightX(other.mAnaRightX)
  , mAnaRightY(other.mAnaRightY)
  , mAnaLeftTrigger(other.mAnaLeftTrigger)
  , mAnaRightTrigger(other.mAnaRightTrigger)
  , mAnaLeftTriggerP(other.mAnaLeftTriggerP)
  , mAnaRightTriggerP(other.mAnaRightTriggerP)
  , btns1(other.btns1)
  , btns2(other.btns2)
  , btns3(other.btns3)
  , btns4(other.btns4)
#endif
  {
  }
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CFinalInput();
  CFinalInput(const CFinalInputInfo& info, float leftDiv, float rightDiv);
#else
  void InitializeAnalog(float leftDiv, float rightDiv);
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CFinalInput(int channel, float dt, const CControllerData& data, uint inputType);
  CFinalInput(const int channel, const float dt, const CControllerGamepadData& data,
              const float leftDiv, const float rightDiv);
  CFinalInput(int channel, float dt, const COsContext& context);
#else
  CFinalInput ScaleAnalogueSticks(float leftDiv, float rightDiv) const;
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION < RSTL_R3IJ
  CFinalInput(const CFinalInput& other)
  : mDt(other.mDt)
  , mControllerIdx(other.mControllerIdx)
  , mAnaLeftX(other.mAnaLeftX)
  , mAnaLeftY(other.mAnaLeftY)
  , mAnaRightX(other.mAnaRightX)
  , mAnaRightY(other.mAnaRightY)
  , mAnaLeftTrigger(other.mAnaLeftTrigger)
  , mAnaRightTrigger(other.mAnaRightTrigger)
  , mEnableAnaLeftXP(other.mEnableAnaLeftXP)
  , mEnableAnaLeftYP(other.mEnableAnaLeftYP)
  , mEnableAnaRightXP(other.mEnableAnaRightXP)
  , mEnableAnaRightYP(other.mEnableAnaRightYP)
  , mAnaLeftTriggerP(other.mAnaLeftTriggerP)
  , mAnaRightTriggerP(other.mAnaRightTriggerP)
  , btns1(other.btns1)
  , btns2(other.btns2)
  , btns3(other.btns3) {}
#endif
#else
  float DeltaTime() const { return mDt; }
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static CFinalInput NoInput(int channel, float dt);
#else
  void SetDeltaTime(float dt) { mDt = dt; }
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  float Time() const { return mDt; }
  void SetTime(float dt) { mDt = dt; }
#else
  float GetAnalogLeftX() const;
  float GetAnalogLeftY() const;
  float GetAnalogRightX() const;
  float GetAnalogRightY() const;
#endif

  int ControllerNumber() const { return mControllerIdx; }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_R3IJ
  uint GetInputType() const { return mInputType; }
  const CControllerData& GetControllerData() const { return mControllerData; }

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

  // For allowing the old code to compile
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

#else
  void InitializeAnalog(float leftDiv, float rightDiv);

  CFinalInput ScaleAnalogueSticks(float leftDiv, float rightDiv) const;

  float ALAUp() const { return mAnaLeftY > 0.f ? mAnaLeftY : 0.f; }
  float ALADown() const { return mAnaLeftY < 0.f ? -mAnaLeftY : 0.f; }
  float ALALeft() const { return mAnaLeftX < 0.f ? -mAnaLeftX : 0.f; }
  float ALARight() const { return mAnaLeftX > 0.f ? mAnaLeftX : 0.f; }
  float ARAUp() const { return mAnaRightY > 0.f ? mAnaRightY : 0.f; }
  float ARADown() const { return mAnaRightY < 0.f ? -mAnaRightY : 0.f; }
  float ARALeft() const { return mAnaRightX < 0.f ? -mAnaRightX : 0.f; }
  float ARARight() const { return mAnaRightX > 0.f ? mAnaRightX : 0.f; }
  float ALTrigger() const { return mAnaLeftTrigger; }
  float ARTrigger() const { return mAnaRightTrigger; }
  float ADPUp() const { return mB31_DPUp ? 1.f : 0.f; }
  float ADPDown() const { return mB25_DPDown ? 1.f : 0.f; }
  float ADPLeft() const { return mB26_DPLeft ? 1.f : 0.f; }
  float ADPRight() const { return mB24_DPRight ? 1.f : 0.f; }
  float AA() const { return mB24_A ? 1.f : 0.f; }
  float AB() const { return mB25_B ? 1.f : 0.f; }
  float AX() const { return mB26_X ? 1.f : 0.f; }
  float AY() const { return mB27_Y ? 1.f : 0.f; }
  float AZ() const { return mB28_Z ? 1.f : 0.f; }
  float AL() const { return mB29_L ? 1.f : 0.f; }
  float AR() const { return mB30_R ? 1.f : 0.f; }
  float AStart() const { return mB27_Start ? 1.f : 0.f; }
  float ALeftX() const { return mAnaLeftX; }
  float ALeftY() const { return mAnaLeftY; }
  bool DLAUp() const { return kInput_AnalogOnThreshhold < mAnaLeftY ? true : false; }
  bool DLADown() const { return -kInput_AnalogOnThreshhold > mAnaLeftY ? true : false; }
  bool DLALeft() const { return -kInput_AnalogOnThreshhold > mAnaLeftX ? true : false; }
  bool DLARight() const { return kInput_AnalogOnThreshhold < mAnaLeftX ? true : false; }
  bool DRAUp() const { return kInput_AnalogOnThreshhold < mAnaRightY ? true : false; }
  bool DRADown() const { return -kInput_AnalogOnThreshhold > mAnaRightY ? true : false; }
  bool DRALeft() const { return -kInput_AnalogOnThreshhold > mAnaRightX ? true : false; }
  bool DRARight() const { return kInput_AnalogOnThreshhold < mAnaRightX ? true : false; }
  bool DLTrigger() const {
    return kInput_AnalogTriggerOnThreshhold < mAnaLeftTrigger ? true : false;
  }
  bool DRTrigger() const {
    return kInput_AnalogTriggerOnThreshhold < mAnaRightTrigger ? true : false;
  }
  bool DDPUp() const { return mB31_DPUp; }
  bool DDPDown() const { return mB25_DPDown; }
  bool DDPLeft() const { return mB26_DPLeft; }
  bool DDPRight() const { return mB24_DPRight; }
  bool DA() const { return mB24_A; }
  bool DB() const { return mB25_B; }
  bool DX() const { return mB26_X; }
  bool DY() const { return mB27_Y; }
  bool DZ() const { return mB28_Z; }
  bool DL() const { return mB29_L; }
  bool DR() const { return mB30_R; }
  bool DStart() const { return mB27_Start; }
  bool PLAUp() const { return DLAUp() && mEnableAnaLeftYP ? true : false; }
  bool PLADown() const {
    return -kInput_AnalogOnThreshhold > mAnaLeftY && mEnableAnaLeftYP ? true : false;
  }
  bool PLALeft() const {
    return -kInput_AnalogOnThreshhold > mAnaLeftX && mEnableAnaLeftXP ? true : false;
  }
  bool PLARight() const {
    return kInput_AnalogOnThreshhold < mAnaLeftX && mEnableAnaLeftXP ? true : false;
  }
  bool PRAUp() const {
    return kInput_AnalogOnThreshhold < mAnaRightY && mEnableAnaRightYP ? true : false;
  }
  bool PRADown() const {
    return -kInput_AnalogOnThreshhold > mAnaRightY && mEnableAnaRightYP ? true : false;
  }
  bool PRALeft() const {
    return -kInput_AnalogOnThreshhold > mAnaRightX && mEnableAnaRightXP ? true : false;
  }
  bool PRARight() const {
    return kInput_AnalogOnThreshhold < mAnaRightX && mEnableAnaRightXP ? true : false;
  }
  bool PLTrigger() const {
    return kInput_AnalogTriggerOnThreshhold < mAnaLeftTriggerP ? true : false;
  }
  bool PRTrigger() const {
    return kInput_AnalogTriggerOnThreshhold < mAnaRightTriggerP ? true : false;
  }

  bool PDPUp() const { return mB27_PDPUp; }
  bool PDPDown() const { return mB29_PDPDown; }
  bool PDPLeft() const { return mB30_PDPLeft; }
  bool PDPRight() const { return mB28_PDPRight; }
  bool PA() const { return mB28_PA; }
  bool PB() const { return mB29_PB; }
  bool PX() const { return mB30_PX; }
  bool PY() const { return mB31_PY; }
  bool PZ() const { return mB24_PZ; }
  bool PL() const { return mB25_PL; }
  bool PR() const { return mB26_PR; }
  bool PStart() const { return mB31_PStart; }

#endif
#else
  float GetAnalog(int input) const;
  bool GetDigital(int input) const;
  bool GetPressed(int input) const;

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
#if RSTL_PRIME1
  friend class CFinalInputInfo;
#endif

  float mDt;
  int mControllerIdx;
#if !RSTL_PRIME1
#if RSTL_VERSION >= RSTL_R32J
  // The native Wii record is a header followed by the complete device snapshot.
  uint mInputType; // Inferred meaning; native word at offset8.
  CControllerData mControllerData;
#else
  float mAnaLeftX;
  float mAnaLeftY;
  float mAnaRightX;
  float mAnaRightY;
  float mAnaLeftTrigger;
  float mAnaRightTrigger;
  float mAnaLeftTriggerP;
  float mAnaRightTriggerP;

  // Echoes packs Prime's analog press flags into the button bitfield.
  union {
    struct {
      uchar mB24_enableAnaLeftXP : 1;
      uchar mB25_enableAnaLeftYP : 1;
      uchar mB26_enableAnaRightXP : 1;
      uchar mB27_enableAnaRightYP : 1;
      uchar mB28_A : 1;
      uchar mB29_B : 1;
      uchar mB30_X : 1;
      uchar mB31_Y : 1;
    };
    uchar btns1;
  };
  union {
    struct {
      uchar mB24_Z : 1;
      uchar mB25_L : 1;
      uchar mB26_R : 1;
      uchar mB27_DPUp : 1;
      uchar mB28_DPRight : 1;
      uchar mB29_DPDown : 1;
      uchar mB30_DPLeft : 1;
      uchar mB31_Start : 1;
    };
    uchar btns2;
  };
  union {
    struct {
      uchar mB24_PA : 1;
      uchar mB25_PB : 1;
      uchar mB26_PX : 1;
      uchar mB27_PY : 1;
      uchar mB28_PZ : 1;
      uchar mB29_PL : 1;
      uchar mB30_PR : 1;
      uchar mB31_PDPUp : 1;
    };
    uchar btns3;
  };
  union {
    struct {
      uchar mB24_PDPRight : 1;
      uchar mB25_PDPDown : 1;
      uchar mB26_PDPLeft : 1;
      uchar mB27_PStart : 1;
    };
    uchar btns4;
  };
#endif
#endif

#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_R3IJ
  uint mInputType;
  CControllerData mControllerData;
#else

  float mAnaLeftX;
  float mAnaLeftY;
  float mAnaRightX;
  float mAnaRightY;
  float mAnaLeftTrigger;
  float mAnaRightTrigger;

  bool mEnableAnaLeftXP;
  bool mEnableAnaLeftYP;
  bool mEnableAnaRightXP;
  bool mEnableAnaRightYP;

  float mAnaLeftTriggerP;
  float mAnaRightTriggerP;

  union {
    struct {
      uchar mB24_A : 1;
      uchar mB25_B : 1;
      uchar mB26_X : 1;
      uchar mB27_Y : 1;
      uchar mB28_Z : 1;
      uchar mB29_L : 1;
      uchar mB30_R : 1;
      uchar mB31_DPUp : 1;
    };
    uchar btns1;
  };
  union {
    struct {
      uchar mB24_DPRight : 1;
      uchar mB25_DPDown : 1;
      uchar mB26_DPLeft : 1;
      uchar mB27_Start : 1;
      uchar mB28_PA : 1;
      uchar mB29_PB : 1;
      uchar mB30_PX : 1;
      uchar mB31_PY : 1;
    };
    uchar btns2;
  };
  union {
    struct {
      uchar mB24_PZ : 1;
      uchar mB25_PL : 1;
      uchar mB26_PR : 1;
      uchar mB27_PDPUp : 1;
      uchar mB28_PDPRight : 1;
      uchar mB29_PDPDown : 1;
      uchar mB30_PDPLeft : 1;
      uchar mB31_PStart : 1;
    };
    uchar btns3;
  };
#endif
#endif

};
#if !RSTL_PRIME1
#ifdef __MWERKS__
#pragma cpp_extensions reset
#endif
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_R3IJ
CHECK_SIZEOF(CFinalInput, 0x218)
#else
CHECK_SIZEOF(CFinalInput, 48)
#endif
#else
DECLARE_FULL_SIZE_FOR(CFinalInput, RSTL_VERSION >= RSTL_R32J ? 0x218 : 0x2c)
CHECK_SIZEOF(CFinalInput, CFinalInput_FULL_SIZE)
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION < RSTL_R3IJ && defined(__MWERKS__)
#pragma cpp_extensions reset
#endif
#else
#if RSTL_VERSION >= RSTL_R32J
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
#else
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

inline float CFinalInput::ADPUp() const { return mB27_DPUp ? 1.f : 0.f; }

inline float CFinalInput::ADPDown() const { return mB29_DPDown ? 1.f : 0.f; }

inline float CFinalInput::ADPLeft() const { return mB30_DPLeft ? 1.f : 0.f; }

inline float CFinalInput::ADPRight() const { return mB28_DPRight ? 1.f : 0.f; }

inline float CFinalInput::AA() const { return mB28_A ? 1.f : 0.f; }

inline float CFinalInput::AB() const { return mB29_B ? 1.f : 0.f; }

inline float CFinalInput::AX() const { return mB30_X ? 1.f : 0.f; }

inline float CFinalInput::AY() const { return mB31_Y ? 1.f : 0.f; }

inline float CFinalInput::AZ() const { return mB24_Z ? 1.f : 0.f; }

inline float CFinalInput::AL() const { return mB25_L ? 1.f : 0.f; }

inline float CFinalInput::AR() const { return mB26_R ? 1.f : 0.f; }

inline float CFinalInput::AStart() const { return mB31_Start ? 1.f : 0.f; }

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
    return mB27_DPUp;
  }

inline bool CFinalInput::DDPDown() const {
    return mB29_DPDown;
  }

inline bool CFinalInput::DDPLeft() const {
    return mB30_DPLeft;
  }

inline bool CFinalInput::DDPRight() const {
    return mB28_DPRight;
  }

inline bool CFinalInput::DA() const { return mB28_A; }

inline bool CFinalInput::DB() const { return mB29_B; }

inline bool CFinalInput::DX() const { return mB30_X; }

inline bool CFinalInput::DY() const { return mB31_Y; }

inline bool CFinalInput::DZ() const { return mB24_Z; }

inline bool CFinalInput::DL() const { return mB25_L; }

inline bool CFinalInput::DR() const { return mB26_R; }

inline bool CFinalInput::DStart() const { return mB31_Start; }

inline bool CFinalInput::PLAUp() const {
    return kInput_AnalogOnThreshhold < mAnaLeftY && mB25_enableAnaLeftYP ? true : false;
  }

inline bool CFinalInput::PLADown() const {
    return -kInput_AnalogOnThreshhold > mAnaLeftY && mB25_enableAnaLeftYP ? true : false;
  }

inline bool CFinalInput::PLALeft() const {
    return -kInput_AnalogOnThreshhold > mAnaLeftX && mB24_enableAnaLeftXP ? true : false;
  }

inline bool CFinalInput::PLARight() const {
    return kInput_AnalogOnThreshhold < mAnaLeftX && mB24_enableAnaLeftXP ? true : false;
  }

inline bool CFinalInput::PRAUp() const {
    return kInput_AnalogOnThreshhold < mAnaRightY && mB27_enableAnaRightYP ? true : false;
  }

inline bool CFinalInput::PRADown() const {
    return -kInput_AnalogOnThreshhold > mAnaRightY && mB27_enableAnaRightYP ? true : false;
  }

inline bool CFinalInput::PRALeft() const {
    return -kInput_AnalogOnThreshhold > mAnaRightX && mB26_enableAnaRightXP ? true : false;
  }

inline bool CFinalInput::PRARight() const {
    return kInput_AnalogOnThreshhold < mAnaRightX && mB26_enableAnaRightXP ? true : false;
  }

inline bool CFinalInput::PLTrigger() const {
    return kInput_AnalogTriggerOnThreshhold < mAnaLeftTriggerP ? true : false;
  }

inline bool CFinalInput::PRTrigger() const {
    return kInput_AnalogTriggerOnThreshhold < mAnaRightTriggerP ? true : false;
  }

inline bool CFinalInput::PDPUp() const { return mB31_PDPUp; }

inline bool CFinalInput::PDPDown() const { return mB25_PDPDown; }

inline bool CFinalInput::PDPLeft() const {
    return mB26_PDPLeft;
  }

inline bool CFinalInput::PDPRight() const {
    return mB24_PDPRight;
  }

inline bool CFinalInput::PA() const {
    return mB24_PA;
  }

inline bool CFinalInput::PB() const {
    return mB25_PB;
  }

inline bool CFinalInput::PX() const { return mB26_PX; }

inline bool CFinalInput::PY() const { return mB27_PY; }

inline bool CFinalInput::PZ() const { return mB28_PZ; }

inline bool CFinalInput::PL() const { return mB29_PL; }

inline bool CFinalInput::PR() const { return mB30_PR; }

inline bool CFinalInput::PStart() const { return mB27_PStart; }
#endif
#endif

#endif // _CFINALINPUT
