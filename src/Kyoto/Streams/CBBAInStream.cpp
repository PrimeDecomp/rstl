#include "Kyoto/Streams/CBBAInStream.hpp"

#include "Corruption/Kyoto/Network/CBBASupport.hpp"
#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "rstl/auto_ptr.hpp"

CInputStream::SBufferAndSize CBBAInStream::LoadFileBuffer(const char* path) {
  // Guessed name. Closes the host file handle when the load is done, whichever way it ends.
  struct SHostFileGuard {
    SHostFileGuard() : mHandle(-1) {}
    ~SHostFileGuard() {
      if (mHandle != -1) {
        CBBASupport::BBAClose(mHandle);
      }
    }

    int mHandle;
  };

  SHostFileGuard file;
  int error = CBBASupport::BBAOpen(path, kBOM_Read, &file.mHandle);
  RS_VERIFY_THROW_DIRECT(34, error == kFE_OK, false,
                         CBasics::Stringize("Could not open BBA file - '%s'\n", path))
  int size = 0;
  error = CBBASupport::BBAGetFileSize(file.mHandle, &size);
  RS_VERIFY_THROW_DIRECT(38, error == kFE_OK, false,
                         CBasics::Stringize("Could not get size of BBA file - '%s'\n", path))
  rstl::auto_ptr< uchar > data(static_cast< uchar* >(
      CMemory::Alloc(size, IAllocator::kHI_RoundUpLen, IAllocator::kSC_Unk1, IAllocator::kTP_Heap,
                     CCallStack(-1, "CBBAInStream.cpp(39) : ", kUnknownType))));
  error = CBBASupport::BBARead(file.mHandle, data.get(), size);
  RS_VERIFY_THROW_DIRECT(42, error == kFE_OK, false,
                         CBasics::Stringize("Could not read BBA file - '%s'\n", path))
  return SBufferAndSize(data.release(), size);
}

CBBAInStream::CBBAInStream(const char* path) : CInputStream(LoadFileBuffer(path), true) {}

CBBAInStream::~CBBAInStream() {}

uint CBBAInStream::GetRemainingBytes() const {
  return mBufferSize - (mpBufferAt.p8 - mpBufferStart);
}
