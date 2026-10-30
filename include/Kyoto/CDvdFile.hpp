#ifndef _CDVDFILE
#define _CDVDFILE
#include "rstl/RstlVersions.h"

#include "Kyoto/CDvdRequest.hpp"
#include "types.h"

#include "rstl/auto_ptr.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/string.hpp"

struct DVDFileInfo;

enum ESeekOrigin {
  kSO_Begin, // Metaforce guess, alternate guess kSO_Set
  kSO_Cur, // Metaforce guess, alternate guess kSO_Current
  kSO_End, // Metaforce guess
};

class CDvdFile;
struct CDvdFileARAM;
class CDvdFile {
public:
  CDvdFile(const char* name);
  ~CDvdFile();
#if !RSTL_PRIME1
  const rstl::string& GetFilename() const { return mFilename; }
#endif

  uint Length() { return mSize; }
  void HandleDVDInterrupt();
  void HandleARAMInterrupt();
  void PingARAMTransfer();
  void TryARAMFile();
  void PushARAMFileLoad();
  void PopARAMFileLoad();
  bool IsARAMFileLoaded();
#if !RSTL_PRIME1
  bool IsARAMFile() const { return mARAMAllocated; }
#endif

  void StartARAMFileLoad();
  void StallForARAMFile();
  CDvdRequest* SyncRead(void* buf, uint len);
  void SyncSeekRead(void* buf, uint len, ESeekOrigin, int offset);
  CDvdRequest* AsyncSeekRead(void* buf, uint len, ESeekOrigin, int offset);
  void CloseFile();
  void CalcFileOffset(int offset, ESeekOrigin origin);
  void UpdateFilePos(int pos);
  const int GetFileSize() const { return mSize; }
#if RSTL_PRIME1
  bool IsARAMFile() const { return mARAMAllocated; }
#endif

  static bool FileExists(const char*);
  static void DVDARAMXferCallback(s32, DVDFileInfo*);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static void ARAMARAMXferCallback(uintptr_t addr);
#else
  static void ARAMARAMXferCallback(u32 addr);
#endif

  static void internalCallback(s32, DVDFileInfo*);
#if RSTL_PRIME1
  const rstl::string& GetFilename() const { return mFilename; }
#endif

private:
  int mFileEntry;
  uchar* mARAMBuffer;
  bool mARAMAllocated;
  bool mARAMPopped;
  rstl::single_ptr< CDvdFileARAM > mARAMFile;
  int mOffset;
  int mSize;
  rstl::string mFilename;
};
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION < RSTL_R3IJ
CHECK_SIZEOF(CDvdFile, 0x28)
#endif
#else
CHECK_SIZEOF(CDvdFile, 0x28)

// Guessed name. Set whenever a DVD read is issued or completes; retail code never reads it.
extern bool gDvdActivity;
#endif

#endif // _CDVDFILE
