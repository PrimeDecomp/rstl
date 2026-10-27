#ifndef KYOTO_STREAMS_CFIOFILESUPPORT_HPP
#define KYOTO_STREAMS_CFIOFILESUPPORT_HPP

#include "Kyoto/Streams/EFileError.hpp"

#include "dolphin/mcc.h"

// The host file layer; every member is static. The calls return an EFileError
// value.
class CFIOFileSupport {
public:
  static bool IsInitialized();
  static int Initialize();
  static int Open(const char* path, int mode, int* handle);
  static int GetFileSize(int handle, unsigned int* size);
  static int Close(int handle);
  // Guessed names. Writes or reads a buffer on an open host file and stores the transferred
  // count when the out pointer is given. kFE_OK means the whole buffer moved.
  static int Write(int handle, const void* data, unsigned int size, unsigned int* written);
  static int Read(int handle, void* data, unsigned int size, unsigned int* read);
  // Guessed name. Seeks an open host file; origin is passed to FIOFseek.
  static int Seek(int handle, int offset, int origin);
  // Guessed name. Tears down the file layer.
  static void Shutdown();
  // Guessed name. MCC (host communication) ready callback; marks the link ready once initialized.
  static void MCCReadyCallback(MCC_SYSEVENT event);
  // Guessed name. Device-select callback; records the selected device and returns zero.
  static int SelectDeviceCallback(s32 device);
};

#endif
