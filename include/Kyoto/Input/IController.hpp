#ifndef _ICONTROLLER
#define _ICONTROLLER
#include "rstl/RstlVersions.h"

#include "types.h"

#if !RSTL_PRIME1
#include "Kyoto/Input/CControllerGamepadData.hpp"
#include "Kyoto/Input/CControllerData.hpp"
#endif

#include "Kyoto/Input/InputTypes.hpp"
#if RSTL_PRIME1
#include "Kyoto/Input/CControllerData.hpp"
#include "Kyoto/Input/CControllerGamepadData.hpp"
#endif

class COsContext;

class IController {
protected:
  static const float kAbsoluteMinimum;
  static const float kAbsoluteMaximum;
  static const float kRelativeMinimum;
  static const float kRelativeMaximum;

public:
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_R3IJ
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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_R3IJ
  virtual void Update(float dt) = 0;
#endif
#else
  // The Wii slots follow Prime's Revolution IController.
#if RSTL_VERSION >= RSTL_R32J
  virtual void Update(float dt) = 0;
#endif
#endif

  virtual uint GetDeviceCount() const = 0;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_R3IJ
  virtual CControllerData& GetInput(uint channel) = 0;
  virtual void SetInput(const CControllerData& input, int channel) = 0;
  virtual EDeviceType GetControllerType(int channel) const = 0;
  virtual void SetMotorEnabled(int channel, bool enabled) = 0;
  virtual void SetMotorState(int channel, EMotorState state) = 0;
  virtual void SetAcceptAdditionalConnections(bool accept) = 0;
  virtual CControllerData::EPointerState GetPointerState(int channel) const = 0;
  virtual uint GetPointerValidFrameCount(int channel) const = 0;
  virtual uint GetPointerInvalidFrameCount(int channel) const = 0;
  virtual CVector2f GetPointerPosition(int channel) const = 0;
  virtual void SetPointerRecenterMode(uint channel, EPointerRecenterMode mode) = 0;
  virtual bool HasMotionActivity(uint channel) const = 0;

  CControllerGamepadData& GetGamepadData(int controller); // To let old code compile

#else
  virtual CControllerGamepadData& GetGamepadData(int controller) = 0;
  virtual uint GetControllerType(int) const = 0;
  virtual void SetMotorState(EIOPort port, EMotorState state) = 0;
#endif
#else
#if RSTL_VERSION >= RSTL_R32J
  virtual CControllerData& GetGamepadData(uint channel) = 0;
  // Temporary GC snapshot declaration for unrecovered Wii source. Native Wii calls
  // use the uint channel overload above; the two snapshot layouts are different.
  CControllerGamepadData& GetGamepadData(int controller);
#else
  virtual CControllerGamepadData& GetGamepadData(int controller) = 0;
#endif
#if RSTL_VERSION >= RSTL_R32J
  virtual void SetInput(int controller) = 0; // Guessed name
#endif
  virtual uint GetControllerType(int) const = 0;
#if RSTL_VERSION >= RSTL_R32J
  virtual void SetMotorEnabled(int channel, bool enabled) = 0; // Guessed name
#endif
  virtual void SetMotorState(EIOPort port, EMotorState state) = 0;
#endif

  static IController* Create(const COsContext& ctx);
};
#if RSTL_PRIME1
CHECK_SIZEOF(IController, 0x4)
#endif

#endif // _ICONTROLLER
