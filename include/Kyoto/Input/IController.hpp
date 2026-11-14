#ifndef _ICONTROLLER
#define _ICONTROLLER
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"

#include "rstl/types.h"

#include "Kyoto/Input/CControllerGamepadData.hpp"
#include "Kyoto/Input/CControllerData.hpp"

#include "Kyoto/Input/InputTypes.hpp"

class COsContext;

class IController {
protected:
  static const float kAbsoluteMinimum;
  static const float kAbsoluteMaximum;
  static const float kRelativeMinimum;
  static const float kRelativeMaximum;

public:
#if RSTL_CONTROLLER_WII
  enum EDeviceType {
    kDT_Wiimote,
    kDT_Nunchuk,
    kDT_Classic,
    kDT_Unsupported,
    kDT_Disconnected,
  };

  enum EPointerRecenterMode {
    kPRM_Hold,
    kPRM_Both,
    kPRM_X,
    kPRM_Y,
  };
#endif

  IController();
  virtual ~IController();
  virtual void Poll() = 0;
#if RSTL_CONTROLLER_WII
  virtual void Update(float dt) = 0;
#endif

  virtual uint GetDeviceCount() const = 0;
#if RSTL_CONTROLLER_WII
  virtual CControllerData& GetInput(uint channel) = 0;
  virtual void SetInput(const CControllerData& input, int channel) = 0;
  virtual EDeviceType GetControllerType(int channel) const = 0;
  virtual void SetMotorEnabled(int channel, bool enabled) = 0;
  virtual void SetMotorState(int channel, EMotorState state) = 0;
#if RSTL_CONTROLLER_EXTENDED_VIRTUALS
  virtual void SetAcceptAdditionalConnections(bool accept) = 0;
#endif
  virtual CControllerData::EPointerState GetPointerState(int channel) const = 0;
  virtual uint GetPointerValidFrameCount(int channel) const = 0;
  virtual uint GetPointerInvalidFrameCount(int channel) const = 0;
  virtual CVector2f GetPointerPosition(int channel) const = 0;
  virtual void SetPointerRecenterMode(uint channel, EPointerRecenterMode mode) = 0;
#if RSTL_CONTROLLER_EXTENDED_VIRTUALS
  virtual bool HasMotionActivity(uint channel) const = 0;
#endif

  CControllerData& GetGamepadData(uint channel) { return GetInput(channel); }
  CControllerGamepadData& GetGamepadData(int controller);
#else
  virtual CControllerGamepadData& GetGamepadData(int controller) = 0;
  virtual uint GetControllerType(int) const = 0;
  virtual void SetMotorState(EIOPort port, EMotorState state) = 0;
#endif

  static IController* Create(const COsContext& ctx);
};
#if RSTL_PRIME1
CHECK_SIZEOF(IController, 0x4)
#endif

#endif // _ICONTROLLER
