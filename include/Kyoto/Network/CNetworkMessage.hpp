#ifndef _CNETWORKMESSAGE
#define _CNETWORKMESSAGE

#include "rstl/static_assert.hpp"
#include "rstl/types.h"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CGuid.hpp"

class CInputStream;
class COutputStream;

// Guessed names
enum EBBAMessageType {
  kMT_String0,
  kMT_String1,
  kMT_HostCommand,
  kMT_ConnectionStatus,
  kMT_String4,
  kMT_NamedData0,
  kMT_NamedData1,
  kMT_NamedData2,
  kMT_Count,
};

// Guessed name
struct SOwnedBuffer {
  SOwnedBuffer() : mOwns(false), mData(nullptr) {}
  SOwnedBuffer(const void* data, bool takeOwnership)
  : mOwns(data != nullptr), mData(const_cast< void* >(data)) {
    if (mOwns && !takeOwnership) {
      mOwns = false;
    }
  }
  SOwnedBuffer(const SOwnedBuffer& other) : mOwns(other.mOwns), mData(other.mData) {
    other.mOwns = false;
  }
  SOwnedBuffer& operator=(const SOwnedBuffer& other) {
    if (&other != this) {
      if (mOwns) {
        CMemory::Free(mData);
      }
      mOwns = other.mOwns;
      mData = other.mData;
      other.mOwns = false;
    }
    return *this;
  }
  ~SOwnedBuffer() {
    if (mOwns) {
      CMemory::Free(mData);
    }
  }

  mutable bool mOwns;
  void* mData;
};

// Guessed name
struct SNetworkMessageBody {
  // Guessed name
  SNetworkMessageBody(const CGuid& sender, const CGuid& guid, EBBAMessageType type, int size,
                      SOwnedBuffer payload);
  // Guessed name
  SNetworkMessageBody(CInputStream& in, const CGuid& receiver);
  void Write(COutputStream& out) const;

  CGuid mSender;
  CGuid mGuid;
  // Guessed name
  EBBAMessageType mType;
  int mSize;
  SOwnedBuffer mPayload;
  CGuid mReceiver;
};
CHECK_SIZEOF(SNetworkMessageBody, 0x40)

#endif // _CNETWORKMESSAGE
