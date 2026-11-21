#ifndef _TTOKENSET
#define _TTOKENSET
#include "rstl/RstlVersions.h"

#include "rstl/types.h"

#include <string.h>

// A null name terminates the table; its token is returned when a name is not found.
#if RSTL_CORRUPTION

// Guessed name
template < typename T >
class TTokenSet {
public:
#else
template < class T >
struct TTokenSet {
#endif
  // Guessed names
  enum ESens {
    kS_IgnoreCase,
    kS_Exact,
  };

  const char* mName; // Guessed name
#if RSTL_CORRUPTION
  T mValue;         // Guessed name
#else
  T mToken;          // Guessed name
#endif

  const char* GetNameByToken(T token) const {
    static const char* skBadString = "__BAD_TOKEN__";
#if RSTL_CORRUPTION
    for (const TTokenSet* it = this; it->mName != nullptr; ++it) {
      if (it->mValue == token) {
        return it->mName;
#else
    for (const TTokenSet* entry = this; entry->mName != nullptr; ++entry) {
      if (entry->mToken == token) {
        return entry->mName;
#endif
      }
    }
    return skBadString;
  }

  T GetToken(const char* name, ESens sens) const {
#if RSTL_CORRUPTION
    const TTokenSet* it = this;
    for (; it->mName != nullptr; ++it) {
#else
    const TTokenSet* entry = this;
    for (; entry->mName != nullptr; ++entry) {
#endif
      bool match = false;
      if (name != nullptr) {
        switch (sens) {
#if RSTL_CORRUPTION
        case kS_Exact:
          match = strcmp(name, it->mName) == 0;
          break;
#endif
        case kS_IgnoreCase: {
          const char* a = name;
#if RSTL_CORRUPTION
          const char* b = it->mName;
#else
          const char* b = entry->mName;
#endif
          for (; *a != 0 && *b != 0; ++a, ++b) {
#if RSTL_CORRUPTION
            const int lowerA = ToLower(*a);
            const int lowerB = ToLower(*b);
            if (lowerA != lowerB) {
#else
            if (ToLower(*a) != ToLower(*b)) {
#endif
              break;
            }
          }
          match = *a == 0 && *b == 0;
          break;
        }
#if !RSTL_CORRUPTION
        case kS_Exact:
          match = strcmp(name, entry->mName) == 0;
          break;
        }
#endif
      }
      if (match) {
#if RSTL_CORRUPTION
          return it->mValue;
#else
        return entry->mToken;
#endif
      }
    }
#if RSTL_CORRUPTION
    }
    return it->mValue;
#else
    return entry->mToken;
#endif
  }

  T GetTokenNoCase(const char* name) const { return GetToken(name, kS_IgnoreCase); }

private:
#if RSTL_CORRUPTION
  static int ToLower(const char c) { return c >= 'A' && c <= 'Z' ? c + 32 : c; } // Guessed name
#else
  static int ToLower(char c) { return c >= 'A' && c <= 'Z' ? c + 32 : c; } // Guessed name
#endif
};

#endif // _TTOKENSET
