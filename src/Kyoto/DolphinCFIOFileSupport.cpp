#include "Kyoto/Streams/CFIOFileSupport.hpp"

#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Basics/CStopwatch.hpp"

#include <stdio.h>

static const uint kInvalidHandle = 0xFFFFFFFF; // Guessed name

static int sExiChannel = -1; // Guessed name
static bool sMCCReady;       // Guessed name
static bool sbFIOInitialized;

int CFIOFileSupport::SelectDeviceCallback(s32 device) {
  sExiChannel = device;
  return 0;
}

void CFIOFileSupport::MCCReadyCallback(MCC_SYSEVENT event) {
  if (event == MCC_SYSEVENT_INITIALIZED) {
    sMCCReady = true;
  }
}

int CFIOFileSupport::Initialize() {
  sExiChannel = -1;
  if (!MCCEnumDevices(SelectDeviceCallback)) {
    return kFE_CantInitialize;
  }
  if (sExiChannel == -1) {
    return kFE_CantInitialize;
  }
  if (!MCCInit(static_cast< MCC_EXI >(sExiChannel), 0, MCCReadyCallback)) {
    return kFE_CantInitialize;
  }
  float start = CStopwatch::GetGlobalTime();
  while (!sMCCReady) {
    if (CStopwatch::GetGlobalTime() - start > 10.f) {
      MCCExit();
      return kFE_TimedOut;
    }
  }
  if (!TTYInit(static_cast< MCC_EXI >(sExiChannel), MCC_CHANNEL_8)) {
    return kFE_CantInitialize;
  }
  if (!FIOInit(static_cast< MCC_EXI >(sExiChannel), MCC_CHANNEL_1, 10)) {
    return kFE_CantInitialize;
  }
  start = CStopwatch::GetGlobalTime();
  while (FIOQuery() * 2 + TTYQuery() != 3) {
    if (CStopwatch::GetGlobalTime() - start > 10.f) {
      Shutdown();
      return kFE_TimedOut;
    }
  }
  sbFIOInitialized = true;
  return kFE_OK;
}

bool CFIOFileSupport::IsInitialized() { return sbFIOInitialized; }

void CFIOFileSupport::Shutdown() {
  FIOExit();
  TTYExit();
  if (sMCCReady) {
    MCCExit();
  }
  sbFIOInitialized = false;
  sMCCReady = false;
}

int CFIOFileSupport::Open(const char* path, int mode, int* handle) {
  RS_VERIFY_THROW(207, sbFIOInitialized, false, "FIO System not initialized!");
  u32 fioMode;
  switch (mode) {
  case 0:
    fioMode = 1;
    break;
  case 1:
    fioMode = 3;
    break;
  case 2: {
    const int created = FIOFopen(path, 0x200);
    if (created == kInvalidHandle) {
      return kFE_CantOpen;
    }
    FIOFclose(created);
    fioMode = 2;
    break;
  }
  default:
    *handle = -1;
    return kFE_InvalidOpenMode;
  }
  *handle = FIOFopen(path, fioMode);
  return *handle == kInvalidHandle ? kFE_CantOpen : kFE_OK;
}

int CFIOFileSupport::Close(int handle) {
  RS_VERIFY_THROW(251, sbFIOInitialized, false, "FIO System not initialized!");
  if (!FIOFclose(handle)) {
    return kFE_CantClose;
  }
  return kFE_OK;
}

int CFIOFileSupport::Read(int handle, void* data, unsigned int size, unsigned int* read) {
  RS_VERIFY_THROW(263, sbFIOInitialized, false, "FIO System not initialized!");
  const u32 count = FIOFread(handle, data, size);
  if (read != nullptr) {
    *read = count;
  }
  if (count == -1) {
    return kFE_CouldntRead;
  }
  return count != size ? kFE_PartialRead : kFE_OK;
}

int CFIOFileSupport::Write(int handle, const void* data, unsigned int size, unsigned int* written) {
  RS_VERIFY_THROW(290, sbFIOInitialized, false, "FIO System not initialized!");
  const u32 count = FIOFwrite(handle, const_cast< void* >(data), size);
  if (written != nullptr) {
    *written = count;
  }
  if (count == -1) {
    return kFE_CouldntWrite;
  }
  return count != size ? kFE_PartialWrite : kFE_OK;
}

int CFIOFileSupport::Seek(int handle, int offset, int origin) { // Guessed name
  RS_VERIFY_THROW(317, sbFIOInitialized, false, "FIO System not initialized!");
  return FIOFseek(handle, offset, origin) == -1 ? kFE_CouldntSeek : kFE_OK;
}

int CFIOFileSupport::GetFileSize(int handle, unsigned int* size) {
  RS_VERIFY_THROW(351, sbFIOInitialized, false, "FIO System not initialized!");
  const int position = FIOFseek(handle, 0, SEEK_SET);
  if (position == -1) {
    return kFE_CouldntSeek;
  }
  *size = FIOFseek(handle, 0, SEEK_END);
  if (*size == -1) {
    return kFE_CouldntSeek;
  }
  return FIOFseek(handle, position, SEEK_SET) != 0 ? kFE_CouldntSeek : kFE_OK;
}

