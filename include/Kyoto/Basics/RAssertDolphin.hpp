#ifndef _RASSERTDOLPHIN
#define _RASSERTDOLPHIN

#include "rstl/RstlVersions.h"
#include "rstl/types.h"

#include "Kyoto/Alloc/CCallStack.hpp"

#include "dolphin/os.h"

#define BUILD_INFO_TAG "!#$MetroidBuildInfo!#$"
// TODO: Add handling for build info
#if !RSTL_KYOTO_BUILD_TIME_STRING
#define BUILD_TIME_DUMMY "PAD_PAD_PAD_PAD_PAD_PAD_PAD_PAD_PAD"
#elif RSTL_KYOTO_BUILD_TIME_PAL
#define BUILD_TIME_DUMMY "Build v1.035 10/27/2004 19:48:17\0AD"
#else
#define BUILD_TIME_DUMMY "Build v1.028 10/18/2004 10:44:32\0AD"
#endif
extern const char* const BuildTime;

#define BUILD_INFO BUILD_INFO_TAG BUILD_TIME_DUMMY
#define BUILD_INFO_TAG_SIZE sizeof(BUILD_INFO_TAG) - 1

// The faulting data address (DAR) parameter.
#if RSTL_KYOTO_REL_ERROR_HANDLER
typedef uint ErrorDataAddress;
#else
typedef int ErrorDataAddress;
#endif

void ErrorHandler(OSError code, OSContext* context, int dsisr, ErrorDataAddress dar);
void SetErrorHandlers();
// Formats the message and reports it through ErrorHandler.
void rs_reallyfatal_printf(const char* format, ...);
void rs_log_assert_failure(const CCallStack* stack, const char* source, int line, const char* kind,
                           const char* condition, const char* message);

#if RSTL_KYOTO_DEBUG_BUILD
// Guessed name
typedef int (*TRAssertDebuggerPrintCallback)(const char* message);
// Guessed name
typedef void (*TRAssertDiagnosticPrintCallback)(const char* format, ...);

extern "C" {
// Name from Prime's map (secondary evidence).
bool rs_debugger_is_active();
// Name from Prime's map (secondary evidence).
TRAssertDiagnosticPrintCallback
rs_set_console_printf_func(TRAssertDiagnosticPrintCallback callback);
// Guessed name
int RAssert_ConsumeDebuggerPrintCallback(const char* message);
// Guessed name
void RAssert_DiscardDiagnosticCallback(const char* format, ...);
// Guessed name
TRAssertDebuggerPrintCallback
RAssert_SetDebuggerPrintCallback(TRAssertDebuggerPrintCallback callback);
}
#endif

// Prints through the debugger callback (debug builds only; retail has no such function).
void rs_debugger_printf(const char* format, ...);

// Name from Prime's map (secondary evidence).
void rs_fatal_printf(const char* format, ...);

void rs_halt_cpu();

#endif // _RASSERTDOLPHIN
