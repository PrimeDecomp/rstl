#include "rstl/RstlVersions.h"
#include "Kyoto/Input/CDolphinController.hpp"
#include "Kyoto/Input/IController.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/COsContext.hpp"

const float IController::kAbsoluteMinimum = -1.f;
const float IController::kAbsoluteMaximum = 1.f;
const float IController::kRelativeMinimum = -1.f;
const float IController::kRelativeMaximum = 1.f;

IController::IController() {}
IController::~IController() {}

IController* IController::Create(const COsContext& ctx) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CDolphinController* cont = rs_new CDolphinController();
#else
  CDolphinController* cont = new CDolphinController();
#endif
  cont->Initialize();
  return cont;
}

