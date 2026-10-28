#include "Kyoto/Alloc/Assert.hpp"

// Guessed name. Prints a formatted assertion report and the backtrace to the screen.
extern "C" void RAssert_PrintFormattedMessageAndBacktraceToScreen(const char* format, ...);

// Guessed name for the two local string helpers.
static inline bool HasText(const char* text) { return text != nullptr && *text != '\0'; }

static inline const char* TextOrEmpty(const char* text) { return HasText(text) ? text : ""; }

extern "C" void rs_log_assert_failure(const CCallStack* stack, const char* source, int line,
                                      const char* kind, const char* condition,
                                      const char* message) {
  RAssert_PrintFormattedMessageAndBacktraceToScreen(
      "****** Assertion Failure ******\n%s(%d): %s%s%s%s%s%s\n", source, line, TextOrEmpty(kind),
      HasText(kind) ? " " : "", TextOrEmpty(condition), HasText(condition) ? "" : "",
      HasText(message) ? ">>> " : "", TextOrEmpty(message));
}
