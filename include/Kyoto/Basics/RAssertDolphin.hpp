#ifndef _RASSERTDOLPHIN
#define _RASSERTDOLPHIN

#include "rstl/RstlVersions.h"
#include "types.h"

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
void rs_debugger_printf(const char* format, ...);

#endif // _RASSERTDOLPHIN
