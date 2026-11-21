#ifndef _CREVOLUTIONCONTROLLER
#define _CREVOLUTIONCONTROLLER
#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"
#endif

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#endif
#include "Kyoto/Input/IController.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Math/CVector2f.hpp"
#endif

#include "rstl/reserved_vector.hpp"
#if !RSTL_CORRUPTION
#include "rstl/single_ptr.hpp"
#endif

#include "revolution/kpad.h"

#if RSTL_CORRUPTION


namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(EMotorState)

template <>
struct reserved_vector_fill_argument< EMotorState > {
  typedef EMotorState type;
};
} // namespace rstl
#else
class CScalarInputFilter;
class CRevolutionMotionProcessor;
#endif

// Some method names are inferred; the layout and virtual order come from Trilogy.
class CRevolutionController : public IController {
public:
#if RSTL_CORRUPTION
  CRevolutionController();
#endif
  ~CRevolutionController() override;
  void Poll() override;
#if !RSTL_CORRUPTION
  void Update(float dt) override;
#endif
  uint GetDeviceCount() const override;
#if RSTL_CORRUPTION
  CControllerGamepadData& GetGamepadData(int controller) override;
  uint GetControllerType(int controller) const override;
  void SetMotorState(EIOPort port, EMotorState state) override;
  int GetTrackingState(int controller) const override;
  uint GetConsecutiveValidTrackingFrames(int controller) const override;
  uint GetConsecutiveInvalidTrackingFrames(int controller) const override;
  CVector2f GetTrackedPosition(int controller) const override;
#else
  CControllerData& GetInput(uint channel) override;
  void SetInput(const CControllerData& input, int channel) override;
  EDeviceType GetControllerType(int channel) const override;
  void SetMotorEnabled(int channel, bool enabled) override;
  void SetMotorState(int channel, EMotorState state) override;
#if RSTL_CONTROLLER_EXTENDED_VIRTUALS
  void SetAcceptAdditionalConnections(bool accept) override;
#endif
  CControllerData::EPointerState GetPointerState(int channel) const override;
  uint GetPointerValidFrameCount(int channel) const override;
  uint GetPointerInvalidFrameCount(int channel) const override;
  CVector2f GetPointerPosition(int channel) const override;
  void SetPointerRecenterMode(uint channel, EPointerRecenterMode mode) override;
#if RSTL_CONTROLLER_EXTENDED_VIRTUALS
  bool HasMotionActivity(uint channel) const override;
  virtual float GetMotionIdleTime(uint channel) const;
  virtual bool HasButtonActivity(uint channel) const;
  virtual float GetButtonIdleTime(uint channel) const;
  virtual bool IsControllerIdle(uint channel) const;
#else
  bool HasMotionActivity(uint channel) const;
  float GetMotionIdleTime(uint channel) const;
  bool HasButtonActivity(uint channel) const;
  float GetButtonIdleTime(uint channel) const;
  bool IsControllerIdle(uint channel) const;
#endif
#endif

#if !RSTL_CORRUPTION
  CRevolutionController();
#if RSTL_CONTROLLER_CHANNEL_LIMIT
  void SetMaxControllers(int count);
#endif
#endif
  bool Initialize();
#if RSTL_CORRUPTION
  float GetAnalogStickMaxValue(EJoyAxis axis) const;
#else
  bool IsPointerValid(int channel) const;
  bool IsPointerDevicePresent(int channel) const;
  WPADInfo GetWpadInfo(int channel) const;
  const KPADStatus& GetKpadStatus(int channel) const;
#endif

private:
#if RSTL_CORRUPTION
  void ProcessInputData();
  void AdjustTrackedPositionScale(int controller); // Guessed name
  void UpdateTrackedPosition(int controller);      // Guessed name
  void ProcessAxis(int controller, EJoyAxis axis, CControllerButton& negative,
                   CControllerButton& positive);
  void ProcessButtons(int controller);
  void ProcessDigitalButton(int controller, CControllerButton& button, uint mask);
  void ProcessAnalogButton(float value, CControllerAxis& axis, CControllerButton& button);
#else
  // Event names inferred from callback writes and deferred dispatch.
  enum EConnectionEvent { kCE_None, kCE_Connected, kCE_Disconnected, kCE_Rejected };
  enum EExtensionEvent { kEE_None, kEE_Wiimote, kEE_Nunchuk, kEE_Classic, kEE_Unsupported };

  // Only construction and copying are observed for this 0x21-byte record.
  struct SUnknownInputData {
    uchar mData[32];
    bool mFlag1 : 1;
    bool mFlag2 : 1;
    bool mFlag3 : 1;

    SUnknownInputData() : mFlag1(false), mFlag2(false), mFlag3(false) {}
  };

  static void* AllocateWpadMemory(u32 size);
  static uchar FreeWpadMemory(void* memory);
  static void WpadInfoCallback(s32 channel, s32 result);
  static void WpadConnectCallback(s32 channel, s32 result);
  static void WpadExtensionCallback(s32 channel, s32 extension);

  void QueueConnectionEvent(int channel, EConnectionEvent event);
  void QueueExtensionEvent(int channel, EExtensionEvent event);
  enum { kActiveChannelCount = RSTL_CONTROLLER_CHANNEL_LIMIT ? 4 : 1 };
  void ProcessConnectionEvents();
  int GetMaxControllers() const {
#if RSTL_CONTROLLER_CHANNEL_LIMIT
    return mMaxControllers;
#else
    return 1;
#endif
  }
  bool AcceptsAdditionalConnections() const {
#if RSTL_CONTROLLER_EXTENDED_VIRTUALS
    return mAcceptAdditionalConnections;
#else
    return false;
#endif
  }
  void ApplyPointerDistanceScale(int channel);
  void UpdatePointerState(int channel);
  void ProcessControllerInput(int channel);
  void UpdateAnalogInput(float threshold, int channel, int axis, CControllerButton& negativeButton,
                         CControllerButton& positiveButton);
  void UpdateContinuousAngleAxis(int channel, int axis);
  void ClearButtonEvents(int channel);
  void UpdateDigitalInput(int channel);
  void UpdateButton(uint heldMask, CControllerButton& button, uint mask, int buttonId);
  void UpdateMotionButton(int channel, CControllerButton& button, uint mask);
  void UpdateMotionButtons(int channel);
  void UpdateSwingButton(int channel, CControllerButton& button, uint mask);
  void UpdateSwingButtons(int channel);
  void UpdateIdleTimes(uint channel, float dt);
  void InitializeController(uint channel);
  void SetControllerType(int channel, EDeviceType type);
  void CopyWpadInfo(int channel);
#endif

  rstl::reserved_vector< KPADStatus, 4 > mStatus;
#if RSTL_CORRUPTION
  rstl::reserved_vector< CControllerGamepadData, 4 > mGamepadStates;
  rstl::reserved_vector< EMotorState, 4 > mMotorStates;
  rstl::reserved_vector< uint, 4 > mDeviceTypes;                 // Guessed name
  rstl::reserved_vector< int, 4 > mTrackingStates;               // Guessed name
  rstl::reserved_vector< CVector2f, 4 > mTrackedPositions;       // Guessed name
  rstl::reserved_vector< float, 4 > mTrackedDistances;           // Guessed name
  rstl::reserved_vector< uint, 4 > mConsecutiveValidFrames;      // Guessed name
  rstl::reserved_vector< uint, 4 > mConsecutiveInvalidFrames;    // Guessed name
#else
  rstl::reserved_vector< EDeviceType, 4 > mControllerTypes;
  rstl::reserved_vector< CControllerData, 4 > mInput;
  rstl::reserved_vector< EPointerRecenterMode, 4 > mPointerRecenterMode;
  rstl::reserved_vector< int, 4 > mPointerReacquireFrames;
  rstl::reserved_vector< rstl::single_ptr< CRevolutionMotionProcessor >, 4 > mMotionProcessors;
  rstl::reserved_vector< SUnknownInputData, 4 > mUnknownInputData;
  rstl::reserved_vector< WPADInfo, 4 > mWpadInfo;
  rstl::reserved_vector< WPADInfo, 4 > mWpadInfoBuf;
  rstl::reserved_vector< float, 4 > mInfoPollTimers;
  rstl::reserved_vector< float, 4 > mMotionIdleTimes;
  rstl::reserved_vector< float, 4 > mButtonIdleTimes;
  rstl::reserved_vector< EConnectionEvent, 4 > mPendingConnectionEvents;
  rstl::reserved_vector< EExtensionEvent, 4 > mPendingExtensionEvents;
  rstl::reserved_vector< rstl::single_ptr< CScalarInputFilter >, 4 > mPointerFilterX;
  rstl::reserved_vector< rstl::single_ptr< CScalarInputFilter >, 4 > mPointerFilterY;
  uint mMotorEnabledFlags : 4;
  float mPointerMinDistance;
  float mPointerMaxDistance;
  float mPointerMinScale;
  float mPointerMaxScale;
#if RSTL_CONTROLLER_CHANNEL_LIMIT
  int mMaxControllers;
#endif
#if RSTL_CONTROLLER_EXTENDED_VIRTUALS
  bool mAcceptAdditionalConnections;
#endif
#endif
};
#if !RSTL_CORRUPTION
CHECK_SIZEOF(CRevolutionController, 0xc8c + 4 * (sizeof(KPADStatus) - 0x84) +
                                         4 * RSTL_CONTROLLER_EXTENDED_VIRTUALS +
                                         4 * RSTL_CONTROLLER_CHANNEL_LIMIT)
#endif

#if RSTL_CORRUPTION
CHECK_SIZEOF(CRevolutionController, 0x688);


#endif
#endif // _CREVOLUTIONCONTROLLER
