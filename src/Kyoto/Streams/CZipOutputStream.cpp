#include "Kyoto/Streams/CZipOutputStream.hpp"
#include "rstl/RstlVersions.h"

#if RSTL_KYOTO_DEBUG_BUILD
#include "Kyoto/Alloc/Assert.hpp"
#endif
#include "rstl/math.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Streams/CZipSupport.hpp"

#if RSTL_KYOTO_ZIP_OUTPUT_FINISH
CZipOutputStream::CZipOutputStream(COutputStream* out, int level)
: COutputStream(1024)
, mOutput(out)
, mCompressedBytesWritten(0)
, mZStream(RSTL_KYOTO_NEW(14) z_stream)
, mFinished(false) {
  mZStream->zalloc = CZipSupport::Alloc;
  mZStream->zfree = CZipSupport::Free;
  mZStream->opaque = nullptr;

  int useLevel = 9;
  if (level <= 9) {
    useLevel = level;
  }
  useLevel = useLevel < 0 ? 0 : useLevel;
  deflateInit_(mZStream.get(), useLevel, "1.1.3", sizeof(z_stream));
}

bool CZipOutputStream::Process(bool finish) {
  unsigned char output[1024];
  mZStream->avail_out = sizeof(output);
  mZStream->next_out = output;
  const int result = deflate(mZStream.get(), finish ? 4 : 0);
#if RSTL_KYOTO_DEBUG_BUILD
  if (result != 0 && result != 1) {
    RS_VERIFY_THROW(40, false, kException_OutputError, "kException_OutputError");
  }
#endif
  const unsigned int remaining = mZStream->avail_out;
  if (sizeof(output) - remaining != 0) {
    mOutput->DoPut(output, sizeof(output) - mZStream->avail_out);
    mCompressedBytesWritten += sizeof(output) - mZStream->avail_out;
  }
  if (result == 1) {
    return true;
  }
  return false;
}

CZipOutputStream::~CZipOutputStream() {
  Finish();
  deflateEnd(mZStream.get());
}

void CZipOutputStream::Finish() {
  DoFlush();
  mZStream->next_in = nullptr;
  mZStream->avail_in = 0;
  while (!Process(true)) {
  }
}

void CZipOutputStream::Write(const void* ptr, size_t len) {
  mZStream->next_in = static_cast< unsigned char* >(const_cast< void* >(ptr));
  mZStream->avail_in = len;
  while (mZStream->avail_in != 0) {
    Process(false);
  }
}

int CZipOutputStream::GetCompressedBytesWritten() {
  if (!mFinished) {
    Finish();
    mFinished = true;
  }
  return mCompressedBytesWritten;
}
#else
CZipOutputStream::CZipOutputStream(COutputStream* out, int level)
: COutputStream(1024)
, mOutput(out)
, mCompressedBytesWritten(0)
, mZStream(rs_new z_stream)
, mUnk(0) {
  mZStream->zalloc = CZipSupport::Alloc;
  mZStream->zfree = CZipSupport::Free;
  mZStream->opaque = nullptr;

  int useLevel = 9;
  if (level < 10) {
    useLevel = level;
  }
  deflateInit(mZStream.get(), level);
}

CZipOutputStream::~CZipOutputStream() {
  DoFlush();
  while (!Process(true))
    ;
}

void CZipOutputStream::Write(const void* ptr, size_t len) {
  mZStream->next_in = static_cast< unsigned char* >(const_cast< void* >(ptr));
  mZStream->avail_in = len;

  while (mZStream->avail_in != 0) {
    Process(false);
  }
}

bool CZipOutputStream::Process(bool finish) {
  Bytef outBuf[1024];
  mZStream->avail_out = 1024;
  mZStream->next_out = outBuf;
  int ret = deflate(mZStream.get(), finish);
  /* assertion failure logic */

  if (mZStream->avail_out != 1024) {
    mOutput->DoPut(outBuf, 1024 - mZStream->avail_out);
    mCompressedBytesWritten += 1024 - mZStream->avail_out;
  }

  return ret == Z_STREAM_END;
}
#endif
