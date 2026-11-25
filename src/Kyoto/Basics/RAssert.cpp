#include "Kyoto/Alloc/Assert.hpp"

#include "Kyoto/Basics/RAssertDolphin.hpp"


// Guessed name for the two local string helpers.
static inline bool HasText(const char* text) { return text != nullptr && *text != '\0'; }

static inline const char* TextOrEmpty(const char* text) { return HasText(text) ? text : ""; }

void rs_log_assert_failure(const CCallStack* stack, const char* source, int line,
                                      const char* kind, const char* condition,
                                      const char* message) {
  rs_fatal_printf(
      "****** Assertion Failure ******\n%s(%d): %s%s%s%s%s%s\n", source, line, TextOrEmpty(kind),
      HasText(kind) ? " " : "", TextOrEmpty(condition), HasText(condition) ? "" : "",
      HasText(message) ? ">>> " : "", TextOrEmpty(message));
}
