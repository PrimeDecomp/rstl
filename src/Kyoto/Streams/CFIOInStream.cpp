#include "Kyoto/Streams/CFIOInStream.hpp"

#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Streams/CFIOFileSupport.hpp"

#include "rstl/auto_ptr.hpp"

// Guessed name. Closes the host file when the read leaves scope.
struct SFileHandle {
  int mHandle;

  SFileHandle() : mHandle(-1) {}
  ~SFileHandle() {
    if (mHandle != -1) {
      CFIOFileSupport::Close(mHandle);
    }
  }
};

// Guessed name. Reads a whole host file into a buffer the stream will own.
static CInputStream::SBufferAndSize ReadFileToOwnedBuffer(const char* path) {
  SFileHandle file;
  int error = CFIOFileSupport::Open(path, 0, &file.mHandle);
  RS_VERIFY_THROW_DIRECT(35, error == kFE_OK, false,
                         CBasics::Stringize("Could not open FIO file - '%s'\n", path));

  unsigned int size = 0;
  error = CFIOFileSupport::GetFileSize(file.mHandle, &size);
  RS_VERIFY_THROW_DIRECT(39, error == kFE_OK, false,
                         CBasics::Stringize("Could not get size of FIO file - '%s'\n", path));

  rstl::auto_ptr< uchar > buffer(static_cast< uchar* >(
      CMemory::Alloc(size, IAllocator::kHI_RoundUpLen, IAllocator::kSC_Unk1, IAllocator::kTP_Heap,
                     CCallStack(-1, "CFIOInStream.cpp(40) : ", kUnknownType))));
  unsigned int read = static_cast< unsigned int >(-1);
  error = CFIOFileSupport::Read(file.mHandle, buffer.get(), size, &read);
  RS_VERIFY_THROW_DIRECT(44, error == kFE_OK, false,
                         CBasics::Stringize("Could not read FIO file - '%s'\n", path));
  return CInputStream::SBufferAndSize(buffer.release(), size);
}

CFIOInStream::CFIOInStream(const char* path) : CInputStream(ReadFileToOwnedBuffer(path), true) {}
