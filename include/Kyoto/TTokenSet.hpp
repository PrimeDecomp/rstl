#ifndef _TTOKENSET
#define _TTOKENSET

#include "types.h"

#include <string.h>

// A null name terminates the table; its token is returned when a name is not found.
template < class T >
struct TTokenSet {
  // Guessed names
  enum ESens {
    kS_IgnoreCase,
    kS_Exact,
  };

  const char* mName; // Guessed name
  T mToken;          // Guessed name

  const char* GetNameByToken(T token) const {
    static const char* skBadString = "__BAD_TOKEN__";
    for (const TTokenSet* entry = this; entry->mName != nullptr; ++entry) {
      if (entry->mToken == token) {
        return entry->mName;
      }
    }
    return skBadString;
  }

  T GetToken(const char* name, ESens sens) const {
    const TTokenSet* entry = this;
    for (; entry->mName != nullptr; ++entry) {
      bool match = false;
      if (name != nullptr) {
        switch (sens) {
        case kS_IgnoreCase: {
          const char* a = name;
          const char* b = entry->mName;
          for (; *a != 0 && *b != 0; ++a, ++b) {
            if (ToLower(*a) != ToLower(*b)) {
              break;
            }
          }
          match = *a == 0 && *b == 0;
          break;
        }
        case kS_Exact:
          match = strcmp(name, entry->mName) == 0;
          break;
        }
      }
      if (match) {
        return entry->mToken;
      }
    }
    return entry->mToken;
  }

  T GetTokenNoCase(const char* name) const { return GetToken(name, kS_IgnoreCase); }

private:
  static int ToLower(char c) { return c >= 'A' && c <= 'Z' ? c + 32 : c; } // Guessed name
};

#endif // _TTOKENSET
