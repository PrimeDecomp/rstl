#include "Kyoto/Network/DolphinCBBACommon.hpp"

#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/CDvdFile.hpp"

#include "rstl/optional_object.hpp"

#include "dolphin/card.h"
#include "dolphin/exi.h"
#include "dolphin/os.h"

#include <string.h>

extern "C" {
// Guessed names
void fn_8041724C(int chan);
void fn_80412F90(int chan, uchar* mac);
void fn_80413000(int chan, uint* netmask);
void fn_804130E0(int chan, uint* gateway);
void fn_80413150(int chan, uint* dns);
void fn_804131A8(int chan, int* linkState);
int fn_804149E8(int chan);
int fn_80421038(void* config);
void fn_80421534();
uint fn_804232AC();
int fn_8041FE84(void* leaseInfo);
}

// Guessed name
struct SBBALeaseInfo {
  uchar mUnknown[0x11c];
  uint mLeaseSeconds;
};

// Guessed name
struct SBBASettings {
  uchar mMacAddress[6];
  OSTime mLeaseExpiry;
  uint mHostAddress;
  uint mServerAddress;
  uint mNetmask;
  uint mGateway;
};

// Guessed name
struct SBBASettingsFile {
  uint mVersion;
  SBBASettings mSettings;
};

// Guessed name
struct SBBANetworkConfig {
  uint mVendor;
  void* (*mAllocate)(uint name, int size);
  void (*mFree)(uint name, void* memory, int size);
  uint mUseDhcp;
  uint mHostAddress;
  uint mNetmask;
  uint mGateway;
  uint mDns;
  uint mUnknown20;
  uint mUnknown24;
  uint mUnknown28;
  uint mUnknown2C[7];
};

CHECK_SIZEOF(SBBALeaseInfo, 0x120)
CHECK_SIZEOF(SBBASettings, 0x20)
CHECK_SIZEOF(SBBASettingsFile, 0x28)

static const uint kBBATypeId = 0x04020200;
static const uint kSettingsVersion = 1;

// Guessed names
static SBBANetworkConfig sNetworkConfig = {0x100, BBA_NetworkAllocateCallback,
                                           BBA_NetworkFreeCallback,
                                           1,     0,
                                           0,     0,
                                           0,     0,
                                           0x1000, 0x1000,
                                           {0, 0, 0, 0, 0, 0, 0}};
// Guessed name
static uint sNoAddress;
static bool sFullAsync;
static SBBASettings sSettings;
static OSMutex sNetworkMutex;
static uchar sCardFile[0x2000] ATTRIBUTE_ALIGN(32);
static uchar sCardWork[CARD_WORKAREA_SIZE] ATTRIBUTE_ALIGN(32);

void* BBA_NetworkAllocateCallback(uint name, int size) {
  if (size > 0) {
    OSLockMutex(&sNetworkMutex);
    void* memory = CMemory::Alloc(
        size, IAllocator::kHI_None, IAllocator::kSC_Unk1, IAllocator::kTP_Heap,
        CCallStack(-1, "DolphinCBBACommon.cpp(44) : ", kUnknownType));
    OSUnlockMutex(&sNetworkMutex);
    return memory;
  }
  return 0;
}

void BBA_NetworkFreeCallback(uint name, void* memory, int size) {
  if (memory != 0 && size > 0) {
    OSLockMutex(&sNetworkMutex);
    CMemory::Free(memory);
    OSUnlockMutex(&sNetworkMutex);
  }
}

bool BBA_InitializeNetworkWithSettings(SBBASettings* settings) {
  uint netmask;
  uint gateway;
  uint dns;
  int linkState;
  u32 adapterType;
  uchar mac[6];
  SBBALeaseInfo lease;
  int started;
  uint hostAddress;
  int usedDhcp = 0;
  int result = 0;
  uint attempts = 0;

  if (!EXIGetType(0, 2, &adapterType) || adapterType != kBBATypeId) {
    rs_debugger_printf("BBA adapter not found.\n");
    return false;
  }

  fn_8041724C(0);
  fn_80412F90(0, mac);
  if (memcmp(mac, settings->mMacAddress, sizeof(mac)) == 0) {
    sNetworkConfig.mUseDhcp = 0;
    sNetworkConfig.mHostAddress = settings->mHostAddress;
    sNetworkConfig.mNetmask = settings->mNetmask;
    sNetworkConfig.mGateway = settings->mGateway;
  } else {
    sNetworkConfig.mUseDhcp = 1;
    usedDhcp = 1;
    sNetworkConfig.mHostAddress = 0;
    sNetworkConfig.mNetmask = 0;
    sNetworkConfig.mGateway = 0;
  }

  if (fn_80421038(&sNetworkConfig) >= 0) {
    started = 1;
  } else {
    started = 0;
  }

  while (attempts != 2) {
    hostAddress = fn_804232AC();
    if (hostAddress != 0) {
      ++attempts;
    }
    fn_80413000(0, &netmask);
    fn_804130E0(0, &gateway);
    fn_80412F90(0, mac);
    fn_80413150(0, &dns);
    result = fn_804149E8(0);
    if (result >= 0) {
      continue;
    }

    switch (result) {
    case -100:
      rs_debugger_printf("Connection Failed (%d):", result);
      rs_debugger_printf("Could not find any DHCP server.\n");
      break;
    case -101:
      rs_debugger_printf("Connection Terminated (%d):", result);
      rs_debugger_printf("DHCP Expired.\n");
      break;
    case -102:
      rs_debugger_printf("Connection Failed (%d):", result);
      rs_debugger_printf("DHCP Nak.\n");
      break;
    case -103:
      rs_debugger_printf("Connection Failed (%d):", result);
      rs_debugger_printf("Could not find any access concentrator.\n");
      break;
    case -104:
      rs_debugger_printf("Connection Failed (%d):", result);
      rs_debugger_printf("The requested service-name request could not be honored.\n");
      break;
    case -105:
      rs_debugger_printf("Connection Failed (%d):", result);
      rs_debugger_printf("The access concentrator experienced some error.\n");
      break;
    case -106:
      rs_debugger_printf("Connection Failed (%d):", result);
      rs_debugger_printf("Genric PPPoE error.\n");
      break;
    case -107:
      rs_debugger_printf("Connection Failed (%d):", result);
      rs_debugger_printf("LCP negotiation error.\n");
      break;
    case -108:
      rs_debugger_printf("Connection Failed (%d):", result);
      rs_debugger_printf("Authentication error.\n");
      break;
    case -109:
      rs_debugger_printf("Connection Failed (%d):", result);
      rs_debugger_printf("IPCP negotiation error.\n");
      break;
    case -110:
      rs_debugger_printf("Connection Terminated (%d):", result);
      rs_debugger_printf("The session is terminated by the access concentrator.\n");
      break;
    default:
      rs_debugger_printf("Connection Failed (%d)\n", result);
      break;
    }

    if (adapterType != kBBATypeId) {
      rs_debugger_printf("Broadband adapter is not attached.\n");
    } else {
      fn_804131A8(0, &linkState);
      if (linkState == 0) {
        rs_debugger_printf("Check the LAN cable.\n");
      }
    }

    if (started) {
      fn_80421534();
    }
    ++attempts;
  }

  if (usedDhcp && fn_8041FE84(&lease)) {
    OSTime now = OSGetTime();
    OSTime leaseTicks = OSSecondsToTicks(static_cast< OSTime >(lease.mLeaseSeconds));
    memcpy(settings->mMacAddress, mac, sizeof(mac));
    settings->mLeaseExpiry = now + leaseTicks;
    settings->mHostAddress = hostAddress;
    settings->mNetmask = netmask;
    settings->mGateway = gateway;
  }
  return result >= 0;
}

bool BBA_LoadSettings() {
  SBBASettings unused;
  sFullAsync = BBA_LoadDHCPSettingsFromMemoryCard(1, "BBA_START_ASYNC", &unused) != 0;
  if (sFullAsync) {
    rs_debugger_printf("Found file %s. Using full async connection.\n");
  }
  return BBA_LoadDHCPSettingsFromMemoryCard(1, "BBA_SETTINGS", &sSettings) != 0;
}

bool BBA_IsFullAsyncEnabled() { return sFullAsync; }

ushort BBA_GetServerPort() { return 0x7d02; }

uint BBA_GetServerIPv4Address() { return sSettings.mServerAddress; }

bool BBA_InitializeHardwareFromSettings() {
  bool resetLease = false;
  OSTime now = OSGetTime();
  char serverFile[13] = "bbaSvrIp.dat";
  rstl::optional_object< uint > serverAddress;

  if (CDvdFile::FileExists(serverFile)) {
    CDvdFile file(serverFile);
    uchar raw[0x40];
    uchar* buffer = reinterpret_cast< uchar* >((reinterpret_cast< uint >(raw) + 0x1f) & ~0x1f);
    serverAddress = sNoAddress;
    file.BlockingRead(buffer, 0x20);
    uchar* address = reinterpret_cast< uchar* >(&serverAddress.data());
    address[0] = buffer[0];
    address[1] = buffer[1];
    address[2] = buffer[2];
    address[3] = buffer[3];
  }

  if (!BBA_LoadSettings()) {
    if (!serverAddress.valid()) {
      rs_debugger_printf("Couldn't get network settings from memory card or disk. No BBA "
                         "initialization.\n");
      return false;
    }
    sSettings.mLeaseExpiry = OSGetTime() - OSSecondsToTicks(3600LL);
    sSettings.mNetmask = 0xFFFFFF00;
    if (sSettings.mGateway == 0) {
      sSettings.mGateway = serverAddress.data();
    }
    resetLease = true;
    rs_debugger_printf("New BBA initialization. Using default settings.\n");
  } else if (sSettings.mLeaseExpiry < now) {
    OSCalendarTime calendar;
    OSReport("**********************\n");
    OSTicksToCalendarTime(sSettings.mLeaseExpiry, &calendar);
    OSReport("DHCP lease expired at %d:%d:%d - %d:%d.\n", calendar.mon, calendar.mday,
             calendar.year, calendar.hour, calendar.min);
    OSTicksToCalendarTime(now, &calendar);
    OSReport("Current Time %d:%d:%d - %d:%d.\n", calendar.mon, calendar.mday, calendar.year,
             calendar.hour, calendar.min);
    OSReport("**********************\n");
    resetLease = true;
  }

  if (resetLease) {
    memset(sSettings.mMacAddress, 0, sizeof(sSettings.mMacAddress));
  }

  if (serverAddress.valid()) {
    rs_debugger_printf("Using bbaSvrIp.dat for server settings.\n");
    sSettings.mServerAddress = serverAddress.data();
  }

  uint server = sSettings.mServerAddress;
  const uchar* octets = reinterpret_cast< const uchar* >(&server);
  rs_debugger_printf("Using %d.%d.%d.%d as server IP.\n", octets[0], octets[1],
                     octets[2], octets[3]);

  bool ok = BBA_InitializeNetworkWithSettings(&sSettings);
  if (ok) {
    if (resetLease) {
      BBA_SaveDHCPSettingsToMemoryCard(1, "BBA_SETTINGS", &sSettings);
    }
    rs_debugger_printf("Internal BBA initialization OK.\n");
  }
  return ok;
}

void BBA_ShutdownHardware() {
  fn_80421534();
  OSReport("BBA Hardware shutdown.\n");
}

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

  int result = CARDMount(chan, sCardWork, 0);
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
  if (sectorSize != sizeof(sCardFile)) {
    OSReport("Sector size %d bytes.\n", sectorSize);
    OSReport("Only memory card sector sizes of %d are supported.\n", (int)sizeof(sCardFile));
    CARDUnmount(chan);
    return false;
  }

  memset(sCardFile, 0, sizeof(sCardFile));
  SBBASettingsFile* file = reinterpret_cast< SBBASettingsFile* >(sCardFile);
  file->mVersion = kSettingsVersion;
  memcpy(&file->mSettings, settings, sizeof(SBBASettings));

  CARDDelete(chan, fileName);
  result = CARDCreate(chan, fileName, sectorSize, &fileInfo);
  if (result < 0) {
    OSReport("CARDCreate failed. (%d)\n", result);
    return false;
  }
  result = CARDWrite(&fileInfo, sCardFile, sizeof(sCardFile), 0);
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

  int result = CARDMount(chan, sCardWork, 0);
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
  if (sectorSize != sizeof(sCardFile)) {
    OSReport("Sector size %d bytes.\n", sectorSize);
    OSReport("Only memory card sector sizes of %d are supported.\n", (int)sizeof(sCardFile));
    CARDUnmount(chan);
    return false;
  }

  result = CARDOpen(chan, fileName, &fileInfo);
  if (result < 0) {
    OSReport("CARDOpen failed. (%d)\n", result);
    CARDUnmount(chan);
    return false;
  }

  memset(sCardFile, 0, sizeof(sCardFile));
  result = CARDRead(&fileInfo, sCardFile, sizeof(sCardFile), 0);
  if (result < 0) {
    OSReport("CARDRead failed. (%d)\n", result);
    return false;
  }

  const SBBASettingsFile* file = reinterpret_cast< const SBBASettingsFile* >(sCardFile);
  if (file->mVersion != kSettingsVersion) {
    OSReport("Version numbers for memcard DHCP info don't match. Deleting old version.");
    versionMismatch = 1;
  } else {
    memcpy(outSettings, &file->mSettings, sizeof(SBBASettings));
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

