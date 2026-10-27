#include "Kyoto/Streams/CBBAOutStream.hpp"

#include "Corruption/Kyoto/Network/CBBASupport.hpp"
#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Basics/CBasics.hpp"

CBBAOutStream::CBBAOutStream(const char* path, int bufferLength)
: COutputStream(bufferLength), mFile(-1) {
  int error = CBBASupport::BBAOpen(path, kBOM_Write, &mFile);
  if (error != kFE_OK) {
    RS_VERIFY_FAILURE_IN("CBBAOutStream.cpp", 15, "0", "kException_CannotOpen",
                         CBasics::Stringize("Could not open file - '%s'\n", path))
  }
}

CBBAOutStream::~CBBAOutStream() {
  Flush();
  if (mFile != -1) {
    CBBASupport::BBAClose(mFile);
  }
}

void CBBAOutStream::Write(const void* ptr, size_t len) {
  int error = CBBASupport::BBAWrite(mFile, ptr, len);
  RS_VERIFY_THROW_DIRECT(29, (error == kFE_OK), false, "Couldn't write in CBBAOutStream::Write.")
}
