#ifndef _CCONTROLLERGAMEPADDATA
#define _CCONTROLLERGAMEPADDATA
#include "rstl/RstlVersions.h"

#include "Kyoto/Input/CControllerAxis.hpp"
#include "Kyoto/Input/CControllerButton.hpp"
#include "Kyoto/Input/InputTypes.hpp"

#if RSTL_PRIME1
#include "types.h"
#endif

class CControllerGamepadData {
public:
  void SetDeviceIsPresent(bool present) { mPresent = present; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const bool DeviceIsPresent() const { return mPresent; }
#else
  bool DeviceIsPresent() const { return mPresent; }

  void SetDeviceJustDisconnected(bool disconnected) { mJustDisconnected = disconnected; }
  bool DeviceJustDisconnected() const { return mJustDisconnected; }
#endif

  const CControllerAxis& GetAxis(EJoyAxis axis) const { return mAxes[axis]; }
  CControllerAxis& GetAxis(EJoyAxis axis) { return mAxes[axis]; }

  const CControllerButton& GetButton(EButton button) const { return mButtons[button]; }
  CControllerButton& GetButton(EButton button) { return mButtons[button]; }

  const CControllerAxis& GetAnalogButton(EAnalogButton button) const {
    return mTriggers[button];
  }
  CControllerAxis& GetAnalogButton(EAnalogButton button) { return mTriggers[button]; }

private:
  bool mPresent;
#if !RSTL_PRIME1
  bool mJustDisconnected;
#endif

  CControllerAxis mAxes[4];
  CControllerAxis mTriggers[2];
  CControllerButton mButtons[12];
};

#endif // _CCONTROLLERGAMEPADDATA
