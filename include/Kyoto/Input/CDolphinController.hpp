#ifndef _CDOLPHINCONTROLLER
#define _CDOLPHINCONTROLLER
#include "rstl/RstlVersions.h"

#include "Kyoto/Input/IController.hpp"

class CDolphinController : public IController {
  static const uint skTypeUnknown;
  static const uint skTypeStandard;
  static const uint skTypeGBA;
  static const uint skTypeWavebird;

public:
  CDolphinController();
  ~CDolphinController() override;
  void Poll() override;
  uint GetDeviceCount() const override;
#if RSTL_CONTROLLER_WII
  void Update(float dt) override;
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
#endif
#else
  CControllerGamepadData& GetGamepadData(int controller) override;
  uint GetControllerType(int) const override;
  void SetMotorState(EIOPort port, EMotorState state) override;
#endif

  bool Initialize();
  float GetAnalogStickMaxValue(EJoyAxis axis) const;

private:
  void ReadDevices();
  void ProcessInputData();
  void ProcessAxis(int controller, EJoyAxis axis);
  void ClearAxis(int controller, EJoyAxis axis);

  void ProcessButtons(int controller);
  void ClearButtons(int controller);

  void ProcessDigitalButton(int value, CControllerButton& button, ushort mapping);
  void ProcessAnalogButton(float value, CControllerAxis& axis);
  PADStatus mStatus[4];
  CControllerGamepadData mGamepadStates[4];
  EMotorState mMotorStates[4];
  uint mControllerTypes[4];
  uint mControllerTypePollTime[4];
  uint mValidControllers;
  uint mInvalidControllers;
  uint x1cc_;
};

#endif // _CDOLPHINCONTROLLER
