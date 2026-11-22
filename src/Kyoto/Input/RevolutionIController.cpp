#include "rstl/RstlVersions.h"
#include "Kyoto/Input/IController.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/COsContext.hpp"
#include "Kyoto/Input/CRevolutionController.hpp"

#if RSTL_KYOTO_WPAD_EMU_CONTROLLER
const float IController::kAbsoluteMinimum = -1.f;
const float IController::kAbsoluteMaximum = 1.f;
const float IController::kRelativeMinimum = -1.f;
const float IController::kRelativeMaximum = 1.f;
#endif

IController::IController() {}

IController::~IController() {}

IController* IController::Create(const COsContext& ctx) {
  return rs_new_line_in("RevolutionIController.cpp", 60) CRevolutionController();
}
