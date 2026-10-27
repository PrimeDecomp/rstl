#include "Kyoto/Streams/CFIOOutStream.hpp"

#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Streams/CFIOFileSupport.hpp"

CFIOOutStream::CFIOOutStream(const char* path, int bufferSize)
: COutputStream(bufferSize), mFile(-1) {
  const int error = CFIOFileSupport::Open(path, 2, &mFile);
  if (error != kFE_OK) {
    RS_VERIFY_FAILURE(15, "0", "kException_CannotOpen",
                      CBasics::Stringize("Could not open file - '%s'\n", path))
  }
}

CFIOOutStream::~CFIOOutStream() {
  Flush();
  if (mFile != -1) {
    CFIOFileSupport::Close(mFile);
  }
}

void CFIOOutStream::Write(const void* data, size_t length) {
  unsigned int written = static_cast< unsigned int >(-1);
  const int error = CFIOFileSupport::Write(mFile, data, length, &written);
  RS_VERIFY_THROW_DIRECT(30, (error == kFE_OK), false, "Couldn't write in CFIOOutStream::Write.");
}
