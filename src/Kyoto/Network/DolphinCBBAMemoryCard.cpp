#include "Kyoto/Network/DolphinCBBACommon.hpp"
#include "rstl/static_assert.hpp"

#include "dolphin/card.h"
#include "dolphin/os.h"

#include <string.h>

// Guessed name
struct SBBASettingsFile {
  uint mVersion;
  uchar mSettings[sizeof(SBBASettings)];
};
CHECK_SIZEOF(SBBASettings, 0x20)
CHECK_OFFSETOF(SBBASettingsFile, mSettings, 0x4)
CHECK_SIZEOF(SBBASettingsFile, 0x24)

static const uint kSettingsVersion = 1;

// Guessed names
uchar gBBACardFile[0x2000] ATTRIBUTE_ALIGN(32);
uchar gBBACardWork[CARD_WORKAREA_SIZE] ATTRIBUTE_ALIGN(32);

// Guessed name
static inline int FindCard(int chan) {
  int result;
  do {
    result = CARDProbeEx(chan, 0, 0);
  } while (result == CARD_RESULT_BUSY);
  if (result == CARD_RESULT_READY) {
    return true;
  }
  OSReport("Can't find card in cardSlot %c.\n", "AB"[chan]);
  return false;
}

bool BBA_SaveDHCPSettingsToMemoryCard(int chan, const char* fileName, const void* settings) {
  CARDFileInfo fileInfo;
  CARDStat stat;
  u32 sectorSize;

  CARDInit();
  if (!FindCard(chan)) {
    return false;
  }

  int result = CARDMount(chan, gBBACardWork, 0);
  if (result < 0) {
    OSReport("CARDMount failed. (%d)\n", result);
    return false;
  }
  switch (result) {
  case CARD_RESULT_READY:
  case CARD_RESULT_BROKEN:
    result = CARDCheck(chan);
    if (result < 0) {
      CARDUnmount(chan);
      OSReport("CARDCheck failed. (%d)\n", result);
      return false;
    }
    break;
  }
  switch (result) {
  case CARD_RESULT_READY:
    break;
  case CARD_RESULT_BROKEN:
  case CARD_RESULT_ENCODING:
    CARDUnmount(chan);
    OSReport("CARDMount failed. (%d)\n", result);
    return false;
  default:
    OSReport("CARDMount failed. (%d)\n", result);
    return false;
  }

  result = CARDGetSectorSize(chan, &sectorSize);
  if (result < 0) {
    OSReport("CARDGetSectorSize() failed. (%d)\n", result);
    return false;
  }
  if (sectorSize != sizeof(gBBACardFile)) {
    OSReport("Sector size %d bytes.\n", sectorSize);
    OSReport("Only memory card sector sizes of %d are supported.\n", (int)sizeof(gBBACardFile));
    CARDUnmount(chan);
    return false;
  }

  memset(gBBACardFile, 0, sizeof(gBBACardFile));
  SBBASettingsFile* file = reinterpret_cast< SBBASettingsFile* >(gBBACardFile);
  file->mVersion = kSettingsVersion;
  memcpy(file->mSettings, settings, sizeof(file->mSettings));

  CARDDelete(chan, fileName);
  result = CARDCreate(chan, fileName, sectorSize, &fileInfo);
  if (result < 0) {
    OSReport("CARDCreate failed. (%d)\n", result);
    return false;
  }
  result = CARDWrite(&fileInfo, gBBACardFile, sizeof(gBBACardFile), 0);
  if (result < 0) {
    OSReport("CARDWrite failed. (%d)\n", result);
    return false;
  }
  result = CARDGetStatus(chan, fileInfo.fileNo, &stat);
  if (result < 0) {
    OSReport("CARDGetStatus failed. (%d)\n", result);
    return false;
  }
  CARDClose(&fileInfo);
  CARDUnmount(chan);
  return true;
}

int BBA_LoadDHCPSettingsFromMemoryCard(int chan, const char* fileName, void* outSettings) {
  int versionMismatch = 0;
  CARDFileInfo fileInfo;
  CARDStat stat;
  u32 sectorSize;

  CARDInit();
  if (!FindCard(chan)) {
    return false;
  }

  int result = CARDMount(chan, gBBACardWork, 0);
  if (result < 0) {
    OSReport("CARDMount failed. (%d)\n", result);
    return false;
  }
  switch (result) {
  case CARD_RESULT_READY:
  case CARD_RESULT_BROKEN:
    result = CARDCheck(chan);
    if (result < 0) {
      CARDUnmount(chan);
      OSReport("CARDCheck failed. (%d)\n", result);
      return false;
    }
    break;
  }
  switch (result) {
  case CARD_RESULT_READY:
    break;
  case CARD_RESULT_BROKEN:
  case CARD_RESULT_ENCODING:
    CARDUnmount(chan);
    OSReport("CARDCheck failed. (%d)\n", result);
    return false;
  default:
    OSReport("CARDCheck failed. (%d)\n", result);
    return false;
  }

  result = CARDGetSectorSize(chan, &sectorSize);
  if (result < 0) {
    OSReport("CARDGetSectorSize() failed. (%d)\n", result);
    return false;
  }
  if (sectorSize != sizeof(gBBACardFile)) {
    OSReport("Sector size %d bytes.\n", sectorSize);
    OSReport("Only memory card sector sizes of %d are supported.\n", (int)sizeof(gBBACardFile));
    CARDUnmount(chan);
    return false;
  }

  result = CARDOpen(chan, fileName, &fileInfo);
  if (result < 0) {
    OSReport("CARDOpen failed. (%d)\n", result);
    CARDUnmount(chan);
    return false;
  }

  memset(gBBACardFile, 0, sizeof(gBBACardFile));
  result = CARDRead(&fileInfo, gBBACardFile, sizeof(gBBACardFile), 0);
  if (result < 0) {
    OSReport("CARDRead failed. (%d)\n", result);
    return false;
  }

  const SBBASettingsFile* file = reinterpret_cast< const SBBASettingsFile* >(gBBACardFile);
  if (file->mVersion != kSettingsVersion) {
    OSReport("Version numbers for memcard DHCP info don't match. Deleting old version.");
    versionMismatch = 1;
  } else {
    memcpy(outSettings, file->mSettings, sizeof(file->mSettings));
  }

  result = CARDGetStatus(chan, fileInfo.fileNo, &stat);
  if (result < 0) {
    OSReport("CARDGetStatus failed. (%d)\n", result);
    return false;
  }
  CARDClose(&fileInfo);

  if (versionMismatch) {
    result = CARDDelete(chan, fileName);
    if (result < 0) {
      OSReport("CARDDelete failed. (%d)\n", result);
      return false;
    }
  }

  CARDUnmount(chan);
  OSReport("Successfully loaded DHCP info from memcard.\n");
  return versionMismatch ? 0 : 1;
}
