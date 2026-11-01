#ifndef KYOTO_ALLOC_ASSERT_HPP
#define KYOTO_ALLOC_ASSERT_HPP

#include "Kyoto/Alloc/CCallStack.hpp"
#include "rstl/RstlVersions.h"

// Verify asserts and debugger warnings. Every game's Kyoto has them; only the prototype (a debug
// build) compiles them in.
#if RSTL_KYOTO_DEBUG_BUILD

extern "C" void rs_log_assert_failure(const CCallStack* stack, const char* source, int line,
                                      const char* kind, const char* condition, const char* message);
extern "C" void rs_debugger_printf(const char* format, ...);
extern "C" void rs_halt_cpu();
extern "C" const char kUnknownType[];

// Guessed name. 0x80796BF0: initialized to rs_debugger_printf.
extern void (*gpfnWarningPrintf)(const char* format, ...);

// Guessed name. Reports a message, with optional format arguments, to the debugger and then to
// the warning callback, as every "BUG THIS" site does.
#define RS_DEBUGGER_WARNING(...)                                                                   \
  do {                                                                                             \
    rs_debugger_printf(__VA_ARGS__);                                                               \
    gpfnWarningPrintf(__VA_ARGS__);                                                                \
  } while (0)

// Guessed name. The warning callback followed by the debugger, with a format and optional
// arguments, as the map station "BUG THIS" sites do.
#define RS_WARNING_DEBUGGER_PRINT(...)                                                             \
  do {                                                                                             \
    gpfnWarningPrintf(__VA_ARGS__);                                                                \
    rs_debugger_printf(__VA_ARGS__);                                                               \
  } while (0)

// Guessed name. Only the warning callback, with a format and optional arguments.
#define RS_WARNING_PRINT(...) gpfnWarningPrintf(__VA_ARGS__)

// Guessed name. Only the debugger, with a format and optional arguments.
#define RS_DEBUGGER_PRINT(...) rs_debugger_printf(__VA_ARGS__)

// Guessed name. Debugger first, with the argument also passed to the warning callback.
#define RS_DEBUGGER_WARNING_ARG(message, arg)                                                      \
  do {                                                                                             \
    rs_debugger_printf(message);                                                                   \
    gpfnWarningPrintf(message, arg);                                                               \
  } while (0)

// The failure path every G2MEAB "Verify" assert expands to. The exception is only stringized:
// this build logs "Would have thrown exception" and traps instead of throwing.
// The original macros use __LINE__; the line is explicit here so the original line numbers
// survive in the strings and the rs_log_assert_failure argument. The macro names are guessed.
#define RS_VERIFY_FAILURE(line, conditionText, exceptionText, message)                             \
  RS_VERIFY_FAILURE_IN(__FILE__, line, conditionText, exceptionText, message)

// Same, for sources whose original file name differs from ours (Main.cpp, TOneStatic.h).
#define RS_VERIFY_FAILURE_IN(file, line, conditionText, exceptionText, message)                    \
  {                                                                                                \
    CCallStack stack(0, RS_SOURCE_POSITION(file, line), kUnknownType);                             \
    rs_log_assert_failure(&stack, file, line, "Verify", conditionText, message);                   \
    rs_debugger_printf("Would have thrown exception: %s\n", exceptionText);                        \
    rs_halt_cpu();                                                                                 \
  }

// Variant that tests the condition directly (a compare and branch, no materialized bool),
// as DolphinCGraphics.cpp's arena asserts do. The macro name is guessed.
#define RS_VERIFY_THROW_DIRECT(line, condition, exception, message)                                \
  if (!(condition))                                                                                \
  RS_VERIFY_FAILURE(line, #condition, #exception, message)

// The condition is stored in a bool before the test: CObjectListSmall::AddObject (0x80262294)
// materializes the comparison result rather than branching on it directly.
#define RS_VERIFY_THROW(line, condition, exception, message)                                       \
  {                                                                                                \
    bool verified = (condition);                                                                   \
    if (verified == false)                                                                         \
      RS_VERIFY_FAILURE(line, #condition, #exception, message)                                     \
  }

// Same, for a condition the binary spells differently from the expression compiled (qualified
// enumerators).
#define RS_VERIFY_THROW_AS(line, condition, conditionText, exception, message)                     \
  {                                                                                                \
    bool verified = (condition);                                                                   \
    if (verified == false)                                                                         \
      RS_VERIFY_FAILURE(line, conditionText, #exception, message)                                  \
  }

// Like RS_VERIFY_THROW, reporting the given source file name.
#define RS_VERIFY_THROW_IN(file, line, condition, exception, message)                              \
  {                                                                                                \
    bool verified = (condition);                                                                   \
    if (verified == false)                                                                         \
      RS_VERIFY_FAILURE_IN(file, line, #condition, #exception, message)                            \
  }

// Like RS_VERIFY_THROW_DIRECT, reporting the given source file name.
#define RS_VERIFY_THROW_DIRECT_IN(file, line, condition, exception, message)                       \
  if (!(condition))                                                                                \
  RS_VERIFY_FAILURE_IN(file, line, #condition, #exception, message)
#else
// Builds without asserts keep every assert in the source; each one compiles to nothing and does
// not evaluate its arguments.
#define RS_DEBUGGER_WARNING(...) (void)(0)
#define RS_WARNING_DEBUGGER_PRINT(...) (void)(0)
#define RS_DEBUGGER_WARNING_ARG(message, arg) (void)(0)
#define RS_WARNING_PRINT(...) (void)(0)
#define RS_DEBUGGER_PRINT(...) (void)(0)
#define RS_VERIFY_FAILURE(line, conditionText, exceptionText, message) (void)(0)
#define RS_VERIFY_FAILURE_IN(file, line, conditionText, exceptionText, message) (void)(0)
#define RS_VERIFY_THROW_DIRECT(line, condition, exception, message) (void)(0)
#define RS_VERIFY_THROW(line, condition, exception, message) (void)(0)
#define RS_VERIFY_THROW_AS(line, condition, conditionText, exception, message) (void)(0)
#define RS_VERIFY_THROW_IN(file, line, condition, exception, message) (void)(0)
#define RS_VERIFY_THROW_DIRECT_IN(file, line, condition, exception, message) (void)(0)
#endif

#endif
