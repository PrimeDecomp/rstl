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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1 && RSTL_VERSION >= RSTL_R3IJ
  void Update(float dt) override;
  CControllerData& GetInput(uint channel) override;
  void SetInput(const CControllerData& input, int channel) override;
  EDeviceType GetControllerType(int channel) const override;
  void SetMotorEnabled(int channel, bool enabled) override;
  void SetMotorState(int channel, EMotorState state) override;
  void SetAcceptAdditionalConnections(bool accept) override;
  CControllerData::EPointerState GetPointerState(int channel) const override;
  uint GetPointerValidFrameCount(int channel) const override;
  uint GetPointerInvalidFrameCount(int channel) const override;
  CVector2f GetPointerPosition(int channel) const override;
  void SetPointerRecenterMode(uint channel, EPointerRecenterMode mode) override;
  bool HasMotionActivity(uint channel) const override;
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
#if !RSTL_PRIME1
  void ClearAxis(int controller, EJoyAxis axis);
#endif

  void ProcessButtons(int controller);
#if !RSTL_PRIME1
  void ClearButtons(int controller);
#endif

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
