#ifndef _ICONTROLLER
#define _ICONTROLLER
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"

#include "rstl/types.h"
#include <rstl/types.h>

#include "Kyoto/Input/CControllerGamepadData.hpp"
#if !RSTL_CORRUPTION
#include "Kyoto/Input/CControllerData.hpp"

#endif
#include "Kyoto/Input/InputTypes.hpp"
#include "Kyoto/Math/CVector2f.hpp"

class COsContext;
#if RSTL_CORRUPTION

// Controller type FourCCs, in CRevolutionController.cpp's .sdata2 order.
extern const uint kControllerTypeUnknown; // Guessed name
extern const uint kControllerTypeCube;    // Guessed name
extern const uint kControllerTypeRevn;    // Guessed name
extern const uint kControllerTypeChak;    // Guessed name
#endif

class IController {
protected:
  static const float kAbsoluteMinimum;
  static const float kAbsoluteMaximum;
  static const float kRelativeMinimum;
  static const float kRelativeMaximum;

public:
#if !RSTL_CORRUPTION
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

#endif
  IController();
  virtual ~IController();
  virtual void Poll() = 0;
#if !RSTL_CORRUPTION
#if RSTL_CONTROLLER_WII
  virtual void Update(float dt) = 0;
#endif

#endif
  virtual uint GetDeviceCount() const = 0;
#if RSTL_CORRUPTION
  virtual CControllerGamepadData& GetGamepadData(int controller) = 0;
  virtual uint GetControllerType(int) const = 0;
  virtual void SetMotorState(EIOPort port, EMotorState state) = 0;
  virtual int GetTrackingState(int controller) const = 0;                    // Guessed name
  virtual uint GetConsecutiveValidTrackingFrames(int controller) const = 0;   // Guessed name
  virtual uint GetConsecutiveInvalidTrackingFrames(int controller) const = 0; // Guessed name
  virtual CVector2f GetTrackedPosition(int controller) const = 0;            // Guessed name
#else
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
#endif

  static IController* Create(const COsContext& ctx);
};
#if RSTL_PRIME1
CHECK_SIZEOF(IController, 0x4)
#endif

extern IController* gpController;

#endif // _ICONTROLLER
