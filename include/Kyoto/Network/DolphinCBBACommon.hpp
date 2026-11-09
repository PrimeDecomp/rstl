#ifndef _DOLPHINCBBACOMMON
#define _DOLPHINCBBACOMMON

#include "types.h"

struct SBBASettings;

// Guessed names

int BBA_LoadDHCPSettingsFromMemoryCard(int chan, const char* fileName, void* outSettings);
bool BBA_SaveDHCPSettingsToMemoryCard(int chan, const char* fileName, const void* settings);
void BBA_ShutdownHardware();
bool BBA_InitializeHardwareFromSettings();
uint BBA_GetServerIPv4Address();
ushort BBA_GetServerPort();
bool BBA_IsFullAsyncEnabled();
bool BBA_LoadSettings();
bool BBA_InitializeNetworkWithSettings(SBBASettings* settings);
void BBA_NetworkFreeCallback(uint name, void* memory, int size);
void* BBA_NetworkAllocateCallback(uint name, int size);

#endif // _DOLPHINCBBACOMMON
