#ifndef _COSCONTEXT
#define _COSCONTEXT

#include <stddef.h>

#include "rstl/RstlVersions.h"
#include "types.h"

#include <dolphin/gx/GXStruct.h>

enum EConsoleType {
  kCT_Emulator,
  kCT_Development1,
  kCT_Development2Or3,
  kCT_Retail,
};

class COsKeyState {
public:
  COsKeyState(int key, bool down, bool released, bool repeat, bool unk)
  : mKey(key), mDown(down), mReleased(released), mRepeat(repeat), mUnk(unk) {}

  bool IsPressed() const { return mDown; }
  bool JustPressed() const { return mUnk && mDown; }

private:
  int mKey;
  short mDown : 1;
  short mReleased : 1;
  short mRepeat : 1;
  short mUnk : 1;
};

class COsContext {
public:
  // Guessed name. The string table languages are ENGL, GERM, FREN, SPAN, ITAL, DUTC and JAPN.
  enum ELanguage {
    kLang_English,  // Guessed name
    kLang_German,   // Guessed name
    kLang_French,   // Guessed name
    kLang_Spanish,  // Guessed name
    kLang_Italian,  // Guessed name
    kLang_Dutch,    // Guessed name
    kLang_Japanese, // Guessed name
    kLang_Count,
  };

  // TODO: Once main.cpp is matched make this private
  static bool mProgressiveMode;

  COsContext(bool, bool);
  ~COsContext();

  int OpenWindow(const char* title, int x, int y, int w, int h, bool fullscreen);
  bool Update();
  int GetLanguage() const;
  COsKeyState GetOsKeyState(int key) const;

  void* AllocFromArena(size_t sz);

  uint GetBaseFreeRam() const {
    size_t hiAddr = reinterpret_cast< size_t >(mArenaHi);
    size_t loAddr = reinterpret_cast< size_t >(mArenaLo2);
    return ((hiAddr & ~31) - ((loAddr + 31) & ~31));
  }

#if RSTL_KYOTO_OS_CONTEXT_ARENA_BLOCK
  const GXRenderModeObj& GetRenderModeObj() const { return mRenderMode; }
  void* GetArenaBlock() const { return mArenaBlock; }
  int GetArenaBlockSize() const { return mArenaBlockSize; }
#else
  void* GetFramebuf1() const { return mFrameBuffer1; }
  void* GetFramebuf2() const { return mFrameBuffer2; }
  const GXRenderModeObj& GetRenderModeObj() const { return mRenderMode; }
#endif

  static void SetProgressiveMode(bool progressive) { mProgressiveMode = progressive; }
  static bool GetProgressiveMode() { return mProgressiveMode; }

private:
  int mRight;
  int mBottom;
  int mLeft;
  int mTop;
#if RSTL_KYOTO_OS_CONTEXT_ARENA_BLOCK
  // PAL Prime and Echoes put the console type at 0x10 and OSGetLanguage() at 0x14; the
  // constructor reserves a fixed arena block (0x24/0x2c). Other fields follow Prime.
  int mConsoleType;
  int mLanguage;
#else
  int mFormat;
  int mConsoleType;
#endif
  void* mArenaLo1;
  void* mArenaHi;
  void* mArenaLo2;
#if RSTL_KYOTO_OS_CONTEXT_ARENA_BLOCK
  void* mArenaBlock;
  void* x28_;
  int mArenaBlockSize;
#else
  void* mFrameBuffer1;
  void* mFrameBuffer2;
  int mFrameBufferSize;
#endif
  GXRenderModeObj mRenderMode;
#if RSTL_KYOTO_WII_OS_CONTEXT
  // The Wii context is 0xa8 bytes. Its field order also differs: the Wii constructor writes
  // the MEM2 arena at 0x28-0x30, so the offsets above only hold for GC. Not yet reconstructed;
  // this pad only gives the right size.
  char mWiiUnknown[0x3c];
#endif
};
CHECK_SIZEOF(COsContext, 0x6c)

#endif // _COSCONTEXT
