#include "Kyoto/Basics/RAssertDolphin.hpp"

#include "Kyoto/CRelFileDebugInfo.hpp"
#if RSTL_KYOTO_SCREEN_ASSERTIONS
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CGraphicsSys.hpp"
#include "Kyoto/Text/CFont.hpp"
#endif
#if RSTL_KYOTO_REL_ERROR_HANDLER
#include "dolphin/ai.h"
#include "dolphin/dvd.h"
#include "dolphin/os/OSReset.h"
#else
#include "dolphin/os/OSError.h"
#endif
#include "dolphin/db.h"
#include "dolphin/os/OSMemory.h"
#include "dolphin/pad.h"
#include "dolphin/vi.h"

#include <stdarg.h>
#include <stdio.h>
#if RSTL_KYOTO_SCREEN_ASSERTIONS
#define RASSERT_BUILD_NAME "Release"
static bool sDebuggerChecked; // Guessed name
static bool sDebuggerPresent; // Guessed name
TRAssertDiagnosticPrintCallback gpfnWarningPrintf = rs_debugger_printf;
#else
#define RASSERT_BUILD_NAME "Production"
#endif

#if RSTL_KYOTO_REL_ERROR_HANDLER
#include <string.h>

// Fatal printing formats the message on its own stack; ErrorHandler copies it into buffer.
static bool SkipFatal = false;
#if RSTL_KYOTO_SCREEN_ASSERTIONS
static TRAssertDebuggerPrintCallback sDebuggerPrintCallback; // Guessed name
#endif
static char* rs_debugger_buffer = nullptr;
// Stack frames to leave out of the backtrace, from a "SKIP<n>" message prefix.
static int rs_debugger_size = 0;
#define FATAL_MESSAGE buffer
#define BACKTRACE_FRAMES (rs_debugger_size + 16)
#else
static bool CallFatal = false;
static char rs_debugger_buffer[1024];
static int rs_debugger_buffer_size;
#define SkipFatal CallFatal
#define FATAL_MESSAGE rs_debugger_buffer
#define BACKTRACE_FRAMES 16
#endif
static const GXColor bg = {128, 0, 0, 0};
static const GXColor fg = {255, 255, 255, 0};
#if !RSTL_KYOTO_SCREEN_ASSERTIONS
static const uchar ExitButtons[4] = {PAD_BUTTON_RIGHT, PAD_BUTTON_LEFT, PAD_BUTTON_DOWN,
                                     PAD_BUTTON_UP};
#define INITIAL_COMBO (PAD_BUTTON_X | PAD_BUTTON_Y | PAD_TRIGGER_Z)
#define DPAD (PAD_BUTTON_LEFT | PAD_BUTTON_RIGHT | PAD_BUTTON_UP | PAD_BUTTON_DOWN)

static void hack() {
  static const char* tmp1 = "%s\0\n";
  static const char* tmp2 = "0x%08x: 0x%08x 0x%08x";
#if RSTL_KYOTO_REL_ERROR_HANDLER
  static const char* tmp3 = " 0x%08x: %s";
#endif
}

#endif

#if RSTL_KYOTO_SCREEN_ASSERTIONS
bool RAssert_IsDebuggerPresentCached() {
  if (!sDebuggerChecked) {
    sDebuggerChecked = true;
    sDebuggerPresent = DBIsDebuggerPresent() != 0;
  }
  return sDebuggerPresent;
}

TRAssertDiagnosticPrintCallback
RAssert_SetDiagnosticPrintCallback(TRAssertDiagnosticPrintCallback callback) {
  TRAssertDiagnosticPrintCallback old = gpfnWarningPrintf;
  gpfnWarningPrintf = callback;
  return old;
}

int RAssert_ConsumeDebuggerPrintCallback(const char*) { return 1; }

void RAssert_DiscardDiagnosticCallback(const char*, ...) {}

TRAssertDebuggerPrintCallback
RAssert_SetDebuggerPrintCallback(TRAssertDebuggerPrintCallback callback) {
  TRAssertDebuggerPrintCallback old = sDebuggerPrintCallback;
  sDebuggerPrintCallback = callback;
  return old;
}

void rs_debugger_printf(const char* format, ...) {
  va_list arg;
  char buffer[1024];
  va_start(arg, format);
  vsnprintf(buffer, sizeof(buffer), format, arg);
  va_end(arg);

  if (sDebuggerPrintCallback != nullptr) {
    if (sDebuggerPrintCallback(buffer) == 0) {
      OSReport("%s", buffer);
    }
  } else {
    OSReport("%s", buffer);
  }
}

void rs_halt_cpu() {
  SkipFatal = true;
  asm { opword 0 }
}

void RAssert_PrintFormattedMessageAndBacktraceToScreen(const char* format, ...) {
  va_list arg;
  char buffer[1024];
  va_start(arg, format);
  vsnprintf(buffer, sizeof(buffer), format, arg);
  va_end(arg);
  OSReport("%s", buffer);

  if (CGraphicsSys::IsInitialized() && !RAssert_IsDebuggerPresentCached()) {
    CGraphics::BeginScene();
    const GXRenderModeObj& mode = CGraphics::GetRenderMode();
    CGraphics::SetViewport(0, 0, mode.fbWidth, mode.xfbHeight);
    CGraphics::SetScissor(0, 0, mode.fbWidth, mode.xfbHeight);
    GXSetTevColor(GX_TEVREG0, CColor::Red().GetGXColor());
    CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
    CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
                       GX_TEVPREV);
    CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    CGX::SetTevDirect(GX_TEVSTAGE0);
    CGX::SetNumTevStages(1);
    CGX::SetNumTexGens(1);
    CGX::SetNumChans(0);
    CGX::SetNumIndStages(0);
    CGX::SetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    CGX::SetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
    CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);

    int y = mode.xfbHeight;
    CFont font(1.f);
    for (char* line = strtok(buffer, "\n"); line != nullptr; line = strtok(nullptr, "\n")) {
      int remaining = strlen(line);
      do {
        int count = remaining > 40 ? 40 : remaining;
        char saved = line[count];
        line[count] = '\0';
        y -= font.GetFontSize();
        font.DrawString(line, 0, y, CColor::White());
        remaining -= count;
        line[count] = saved;
        line += count;
      } while (remaining != 0);
    }

    int i = 0;
    u32* frame = reinterpret_cast< u32* >(OSGetStackPointer());
    for (; frame != nullptr && frame != reinterpret_cast< u32* >(-1) && i++ < 16;
         frame = reinterpret_cast< u32* >(*frame)) {
      uint address = frame[1];
      sprintf(buffer, "0x%08x: 0x%08x 0x%08x", frame, *frame, address);
      y -= font.GetFontSize();
      font.DrawString(buffer, 0, y, CColor::White());
      const CRelFileDebugInfo* info = CRelFileDebugInfo::FindByAddress(address);
      if (info != nullptr) {
        sprintf(buffer, " 0x%08x: %s", address - info->GetStart(), info->GetName());
        y -= font.GetFontSize();
        font.DrawString(buffer, 0, y, CColor::White());
      }
    }
    CGraphics::EndScene();
    GXDrawDone();
    GXFlush();
    GXDrawDone();
    VIWaitForRetrace();
    VIWaitForRetrace();
  }
}
#endif

void ErrorHandler(OSError code, OSContext* context, int dsisr, ErrorDataAddress dar) {
#if defined(TARGET_PC)
#if RSTL_KYOTO_REL_ERROR_HANDLER
  const bool hasMessage = rs_debugger_buffer != nullptr && rs_debugger_buffer[0] != '\0';
#else
  const bool hasMessage = rs_debugger_buffer_size > 0;
#endif
  OSReport("Game error %d (DSISR %08x, DAR %08x)%s%s", code, dsisr, dar,
           hasMessage ? ": " : "", hasMessage ? rs_debugger_buffer : "");
#else
#if RSTL_KYOTO_REL_ERROR_HANDLER
  char buffer[1024];
#endif
  OSContext newContext;
  uint loopExitCriteria;
  PADStatus pads[4];
  uchar exitButtons[4];
#if RSTL_KYOTO_REL_ERROR_HANDLER
  u32* tmp;
  uint i;
  u32* gpr;
#else
  u32* gpr;
  uint i;
  uint len;
#endif

  if (code == 15 && ((dsisr & 0x10) != 0 || ((uint)dar > 0x1800000))) {
    return;
  }

  OSProtectRange(3, 0, 0, 3);
#if RSTL_KYOTO_REL_ERROR_HANDLER
  // Silence audio and stop disc reads
  memset((void*)0x81200000, 0, (u32)0x280);
  DCFlushRange((void*)0x81200000, 0x280);
  AIRegisterDMACallback(NULL);
  AIInitDMA(0x81200000, 0x280);
  AIStartDMA();
  DVDCancelAllAsync(nullptr);
#endif
#if !RSTL_KYOTO_SCREEN_ASSERTIONS
  loopExitCriteria = 0;
  PADControlMotor(0, 0);
  VISetBlack(1);
  VIFlush();

#if RSTL_KYOTO_ERROR_HANDLER_NEW_CONTEXT
  // Pivot to a new context for a clean CPU state
  OSClearContext(&newContext);
  OSSetCurrentContext(&newContext);
  OSEnableInterrupts();
#endif

#if RSTL_KYOTO_REL_ERROR_HANDLER
  // Reset the console after 10 seconds without the debug combo held
  OSTime lastTime = OSGetTime();
  OSTime currentTime;
#endif

  // We have an error, lets loop until the player correctly inputs the debug combo:
  // Hold X+Y+Z Followed by pressing right, left, down, up in that order
  while (loopExitCriteria < 4) {
#if RSTL_KYOTO_REL_ERROR_HANDLER
    currentTime = OSGetTime();
    if (OSTicksToSeconds(currentTime - lastTime) >= 10) {
      OSResetSystem(1, 0, 0);
    }
#endif

    PADRead(pads);

#if RSTL_KYOTO_ERROR_HANDLER_NEW_CONTEXT
    // If port 1 is unpopulated, spin until a controller is plugged in
    while (pads[1].err == PAD_ERR_NO_CONTROLLER
#if RSTL_KYOTO_REL_ERROR_HANDLER
           && OSTicksToSeconds(OSGetTime() - lastTime) < 10
#endif
    ) {
      // Reset port 1
      PADReset(PAD_CHAN1_BIT);
      // Get current state
      PADRead(pads);
    }
#endif

    if (pads[1].err == PAD_ERR_NONE) {
      *(uint*)exitButtons = *(uint*)ExitButtons;

      // Check for X + Y + Z to initiate loop exit
      if ((pads[1].button & INITIAL_COMBO) == INITIAL_COMBO) {
#if RSTL_KYOTO_REL_ERROR_HANDLER
        lastTime = OSGetTime();
#endif

        // Is the player pressing any DPAD button?
        if ((pads[1].button & DPAD) != 0) {
          // Ok, check combo order
          if ((pads[1].button & exitButtons[loopExitCriteria]) != 0) {
            loopExitCriteria += 1;

          } else if ((loopExitCriteria != 0) &&
                     ((pads[1].button & exitButtons[loopExitCriteria - 1]) == 0)) {
            // If any of the buttons are unset, or were pressed in the wrong order, reset
            loopExitCriteria = 0;
          }
        }
      } else {
        // Player let go of X+Y+Z, reset loop
        loopExitCriteria = 0;
      }
    }
  }
  VISetBlack(0);
  VIFlush();

#endif

  OSReport("Unhandled exception %d - " RASSERT_BUILD_NAME "\n", code);
  OSReport("%s\n", BuildTime);
  OSReport("------------------------- Context 0x%08x -------------------------\n", context);

  for (i = 0; i < 16; ++i) {
    OSReport("r%-2d  = 0x%08x (%14d)  r%-2d  = 0x%08x (%14d)\n", i, context->gpr[i],
             context->gpr[i], i + 0x10, context->gpr[i + 0x10], context->gpr[i + 0x10]);
  }

  OSReport("LR   = 0x%08x                   CR   = 0x%08x\n", context->lr, context->cr);
  OSReport("SRR0 = 0x%08x                   SRR1 = 0x%08x\n", context->srr0, context->srr1);
  OSReport("DSISR= 0x%08x                   DAR  = 0x%08x\n", dsisr, dar);

#if RSTL_KYOTO_REL_ERROR_HANDLER
  if (rs_debugger_buffer != nullptr) {
    strcpy(buffer, rs_debugger_buffer);
  } else {
    buffer[0] = 0;
  }
  uint len = strlen(buffer);
  if (len != 0) {
    len += sprintf(buffer + len, " - " RASSERT_BUILD_NAME "\n");
  } else {
    len += sprintf(buffer, "Exception %d - " RASSERT_BUILD_NAME "\n", code);
  }
#else
  len = rs_debugger_buffer_size;
  if (rs_debugger_buffer_size != 0) {
    len += sprintf(rs_debugger_buffer + rs_debugger_buffer_size, " - " RASSERT_BUILD_NAME "\n");
  } else {
    len += sprintf(rs_debugger_buffer, "Exception %d - " RASSERT_BUILD_NAME "\n", code);
  }
#endif

  len += sprintf(FATAL_MESSAGE + len, "%s\n", BuildTime);
  len += sprintf(FATAL_MESSAGE + len, "IP: 0x%8.8x  Mem: 0x%8.8x", context->srr0, dar);

  if (code == 15) {
    len += sprintf(FATAL_MESSAGE + len, " - %d", dsisr);
  }

  len += sprintf(FATAL_MESSAGE + len, "\n\n");

  OSReport("\nAddress:      Back Chain    LR Save\n");
  gpr = (u32*)context->gpr[1];
  for (i = 0; gpr != NULL && gpr != (void*)0xFFFFFFFF && i++ < BACKTRACE_FRAMES;) {
    if (gpr > (void*)0x80000400 && gpr < (void*)0x81800000) {
#if RSTL_KYOTO_REL_ERROR_HANDLER
      if (i >= rs_debugger_size) {
        OSReport("0x%08x:   0x%08x    0x%08x\n", gpr, *gpr, gpr[1]);
        u32 iVar2 = gpr[1];
        const CRelFileDebugInfo* tmp = CRelFileDebugInfo::FindByAddress(iVar2);
        if (tmp == NULL) {
          len += sprintf(buffer + len, "0x%08x: 0x%08x 0x%08x\n", gpr, *gpr, gpr[1]);
        }

        if (tmp != NULL) {
          OSReport("RFO:0x%08x: %s\n", iVar2 - tmp->GetStart(), tmp->GetName());
          len += sprintf(buffer + len, "RFO:0x%08x: %s\n", iVar2 - tmp->GetStart(), tmp->GetName());
        }
      }
#else
      OSReport("0x%08x:   0x%08x    0x%08x\n", gpr, *gpr, gpr[1]);
      len += sprintf(rs_debugger_buffer + len, "0x%08x: 0x%08x 0x%08x\n", gpr, *gpr, gpr[1]);
#endif
    } else {
      OSReport("0x%08x:   BAD PTR\n", gpr);
      sprintf(FATAL_MESSAGE + len, "0x%08x: BAD PTR\n", gpr);
      break;
    }
    gpr = (u32*)*gpr;
  }

  OSReport("\nInstruction at 0x%x (read from SRR0) attempted to access invalid address 0x%x (read "
           "from DAR)\n",
           context->srr0, dar);
  if (!SkipFatal) {
    OSFatal(fg, bg, FATAL_MESSAGE);
  }
#endif
}

void SetErrorHandlers() {
#if !defined(TARGET_PC)
  OSSetErrorHandler(OS_ERROR_DSI, (OSErrorHandler)ErrorHandler);
  OSSetErrorHandler(OS_ERROR_ISI, (OSErrorHandler)ErrorHandler);
  OSSetErrorHandler(OS_ERROR_ALIGNMENT, (OSErrorHandler)ErrorHandler);
  OSSetErrorHandler(OS_ERROR_PROTECTION, (OSErrorHandler)ErrorHandler);
#endif
}

void RSTL_KYOTO_FATAL_PRINTF_NAME(const char* format, ...) {
  va_list arg;

#if RSTL_KYOTO_REL_ERROR_HANDLER
  OSProtectRange(3, 0, 0, 3);
  rs_debugger_size = 0;
  if (strncmp(format, "SKIP", 4) == 0) {
    const char* s = format + 4;
    format += 5;
    rs_debugger_size = *s - 0x30;
  }

  char buffer[0x400];
#endif

  va_start(arg, format);
#if RSTL_KYOTO_REL_ERROR_HANDLER
  vsnprintf(buffer, 0x400, format, arg);
#elif NONMATCHING
  rs_debugger_buffer_size = vsnprintf(rs_debugger_buffer, sizeof(rs_debugger_buffer), format, arg);
#else
  rs_debugger_buffer_size = vsprintf(rs_debugger_buffer, format, arg);
#endif
  va_end(arg);
#if RSTL_KYOTO_REL_ERROR_HANDLER
  rs_debugger_buffer = buffer;
#endif

  OSContext context;
  OSSaveContext(&context);
  ErrorHandler(0xff, &context, 0, 0xd1dd0d1e);
}
