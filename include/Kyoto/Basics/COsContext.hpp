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

  // Guessed names, from the Wii system configuration and render consumers.
  enum EAspectRatio { kAR_4x3, kAR_16x9 };
  EAspectRatio GetAspectRatio() const;

  // TODO: Once main.cpp is matched make this private
  static bool mProgressiveMode;

  COsContext(bool, bool);
#if RSTL_KYOTO_OS_CONTEXT_VIRTUAL
  virtual ~COsContext();
#else
  ~COsContext();
#endif

  int OpenWindow(const char* title, int x, int y, int w, int h, bool fullscreen);
  bool Update();
  int GetLanguage() const;
  bool GetEuRgb60Mode() const; // Guessed name.
  COsKeyState GetOsKeyState(int key) const;

  void* AllocFromArena(size_t sz);
  void SetArenaBlockSize(int size); // Guessed name.

  uint GetBaseFreeRam() const {
    size_t hiAddr = reinterpret_cast< size_t >(mArenaHi);
    size_t loAddr = reinterpret_cast< size_t >(mArenaLo2);
    return ((hiAddr & ~31) - ((loAddr + 31) & ~31));
  }

  const GXRenderModeObj& GetRenderModeObj() const { return mRenderMode; }
  void* GetArenaBlock() const;
  int GetArenaBlockSize() const;
  void* GetFramebuf1() const;
  void* GetFramebuf2() const;

  static void SetProgressiveMode(bool progressive) { mProgressiveMode = progressive; }
  static bool GetProgressiveMode() { return mProgressiveMode; }

private:
  void* AllocFromMem1Arena(size_t sz); // Guessed name.

  int mRight;
  int mBottom;
  int mLeft;
  int mTop;
#if RSTL_KYOTO_OS_CONTEXT_ARENA_BLOCK
  // PAL Prime and Echoes retain the language and arena-reservation members.
  // Later contexts add virtual destruction, MEM1 storage and video allocations.
  int mConsoleType;
  int mLanguage;
#else
  int mFormat;
  int mConsoleType;
#endif
#if RSTL_KYOTO_OS_CONTEXT_ASPECT_RATIO
  EAspectRatio mAspectRatio;
#endif
#if RSTL_KYOTO_WII_OS_CONTEXT
  void* mMem1ArenaLo;
  void* mMem1ArenaHi;
  void* mMem1ArenaCurrent;
#endif
  void* mArenaLo1;
  void* mArenaHi;
  void* mArenaLo2;
#if RSTL_KYOTO_OS_CONTEXT_ARENA_BLOCK
  void* mArenaBlock;
#if !RSTL_KYOTO_WII_OS_CONTEXT
  void* x28_;
#endif
  int mArenaBlockSize;
#else
  void* mFrameBuffer1;
  void* mFrameBuffer2;
  int mFrameBufferSize;
#endif
#if RSTL_KYOTO_WII_OS_CONTEXT
  void* mFifo;
  int mFifoSize;
  void* mGpuBuffer;
  int mGpuBufferSize;
  void* mSpareFrameBuffer;
  int mSpareFrameBufferSize;
  void* mSpareTexture;
  GXTexFmt mSpareTextureFormat;
  int mSpareTextureSize;
  void* mFrameBuffer1;
  void* mFrameBuffer2;
  int mFrameBufferSize;
#endif
  // Retained shared storage; the Wii constructor does not initialize this member.
  GXRenderModeObj mRenderMode;
};
#if !RSTL_KYOTO_OS_CONTEXT_VIRTUAL
CHECK_SIZEOF(COsContext, 0x6c)
#endif
#if RSTL_KYOTO_OS_CONTEXT_ARENA_BLOCK
inline void* COsContext::GetArenaBlock() const { return mArenaBlock; }
inline int COsContext::GetArenaBlockSize() const { return mArenaBlockSize; }
#endif
#if !RSTL_KYOTO_OS_CONTEXT_ARENA_BLOCK || RSTL_KYOTO_WII_OS_CONTEXT
inline void* COsContext::GetFramebuf1() const { return mFrameBuffer1; }
inline void* COsContext::GetFramebuf2() const { return mFrameBuffer2; }
#endif
#if RSTL_KYOTO_OS_CONTEXT_ARENA_BLOCK
inline void COsContext::SetArenaBlockSize(int size) { mArenaBlockSize = size; }
#endif
#if RSTL_KYOTO_OS_CONTEXT_ASPECT_RATIO
inline COsContext::EAspectRatio COsContext::GetAspectRatio() const { return mAspectRatio; }
#endif

#endif // _COSCONTEXT
