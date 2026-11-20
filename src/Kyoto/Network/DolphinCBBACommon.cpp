#include "Kyoto/Network/DolphinCBBACommon.hpp"

#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/CDvdFile.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/static_assert.hpp"

#include "dolphin/exi.h"
#include "dolphin/ip/IPEther.h"
#include "dolphin/ip/IPRoute.h"
#include "dolphin/os.h"

#include <string.h>

// Guessed name
struct SBBALeaseInfo {
  uchar mHostAddress[4];
  uchar mNetmask[4];
  uchar mGateway[4];
  uchar mDns1[4];
  uchar mDns2[4];
  uchar mDomain[256];
  ushort mMtu;
  uchar mBroadcastAddress[4];
  u32 mLeaseSeconds;
  uchar mServerAddress[4];
  u32 mRenewalSeconds;
  u32 mRebindingSeconds;
};

// Guessed name
struct SBBAIPv4Address {
  uint mAddress;
};

// Guessed name
struct SBBANetworkConfig {
  ushort mVendor;
  ushort mVersion;
  void* (*mAllocate)(uint name, int size);
  void (*mFree)(uint name, void* memory, int size);
  uint mUseDhcp;
  SBBAIPv4Address mHostAddress;
  SBBAIPv4Address mNetmask;
  SBBAIPv4Address mGateway;
  SBBAIPv4Address mDns1;
  SBBAIPv4Address mDns2;
  s32 mTimeWaitBuffer;
  s32 mReassemblyBuffer;
  s32 mMtu;
  s32 mReceiveWindow;
  OSTime mRetransmitTimeout;
  const char* mPeerId;
  const char* mPassword;
};

extern "C" {
int SOStartup(const SBBANetworkConfig* config);
int SOCleanup();
s32 SOGetHostID();
int DHCPGetStatus(SBBALeaseInfo* info);
}

CHECK_OFFSETOF(SBBALeaseInfo, mLeaseSeconds, 0x11c)
CHECK_SIZEOF(SBBALeaseInfo, 0x12c)
CHECK_SIZEOF(SBBANetworkConfig, 0x48)
CHECK_SIZEOF(SBBASettings, 0x20)

static const uint kBBATypeId = 0x04020200;

// Guessed names
static SBBANetworkConfig sNetworkConfig = {
    0,
    0x100,
    BBA_NetworkAllocateCallback,
    BBA_NetworkFreeCallback,
    1,
    {0}, {0}, {0}, {0}, {0},
    0x1000, 0x1000,
};
// Guessed name
static SBBAIPv4Address sNoAddress = {0};
static bool sFullAsync;
static SBBASettings sSettings;
static OSMutex sNetworkMutex;

void* BBA_NetworkAllocateCallback(uint name, int size) {
  if (size > 0) {
    OSLockMutex(&sNetworkMutex);
    void* memory = CMemory::Alloc(
        size, IAllocator::kHI_None, IAllocator::kSC_Unk1, IAllocator::kTP_Heap,
        CCallStack(-1, "DolphinCBBACommon.cpp(44) : ", gkUnknownType));
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
  s32 mtu;
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

  IFInit(0);
  IPGetMacAddr(0, mac);
  if (memcmp(mac, settings->mMacAddress, sizeof(mac)) == 0) {
    sNetworkConfig.mUseDhcp = 0;
    sNetworkConfig.mHostAddress.mAddress = settings->mHostAddress;
    sNetworkConfig.mNetmask.mAddress = settings->mNetmask;
    sNetworkConfig.mGateway.mAddress = settings->mGateway;
  } else {
    sNetworkConfig.mUseDhcp = 1;
    usedDhcp = 1;
    sNetworkConfig.mHostAddress.mAddress = 0;
    sNetworkConfig.mNetmask.mAddress = 0;
    sNetworkConfig.mGateway.mAddress = 0;
  }

  if (SOStartup(&sNetworkConfig) >= 0) {
    started = 1;
  } else {
    started = 0;
  }

  while (attempts != 2) {
    hostAddress = SOGetHostID();
    if (hostAddress != 0) {
      ++attempts;
    }
    IPGetNetmask(0, reinterpret_cast< uchar* >(&netmask));
    IPGetGateway(0, reinterpret_cast< uchar* >(&gateway));
    IPGetMacAddr(0, mac);
    IPGetMtu(0, &mtu);
    result = IPGetConfigError(0);
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
      IPGetLinkState(0, &linkState);
      if (linkState == 0) {
        rs_debugger_printf("Check the LAN cable.\n");
      }
    }

    if (started) {
      SOCleanup();
    }
    ++attempts;
  }

  if (usedDhcp && DHCPGetStatus(&lease)) {
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
  rstl::optional_object< uint > serverAddress;
  char serverFile[13] = "bbaSvrIp.dat";

  if (CDvdFile::FileExists(serverFile)) {
    CDvdFile file(serverFile);
    uchar raw[0x40];
    uchar* buffer = reinterpret_cast< uchar* >((reinterpret_cast< uint >(raw) + 0x1f) & ~0x1f);
    const uint defaultAddress = sNoAddress.mAddress;
    serverAddress = defaultAddress;
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
  SOCleanup();
  OSReport("BBA Hardware shutdown.\n");
}
