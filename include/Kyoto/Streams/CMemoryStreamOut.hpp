#ifndef _CMEMORYSTREAMOUT
#define _CMEMORYSTREAMOUT

#include "Kyoto/Streams/COutputStream.hpp"

class CMemoryStreamOut : public COutputStream {
public:
  enum EOwnerShip {
    kOS_Owned,
    kOS_NotOwned,
  };

  CMemoryStreamOut(void* buffer, size_t len, EOwnerShip ownerShip = kOS_NotOwned,
                   int blockLen = 4096);
  virtual ~CMemoryStreamOut();

  virtual void Write(const void* ptr, size_t len);

private:
  void* mOutPtr;
  size_t mOutLength;
  size_t mCurrentPosition;
  bool mBufferOwned;
};

CHECK_CHILD_SIZEOF(CMemoryStreamOut, COutputStream, 0x10)

#endif // _CMEMORYSTREAMOUT
