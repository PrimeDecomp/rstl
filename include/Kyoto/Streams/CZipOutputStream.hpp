#ifndef _CZIPOUTPUTSTREAM
#define _CZIPOUTPUTSTREAM

#include "rstl/RstlVersions.h"

#include "Kyoto/Streams/COutputStream.hpp"

#include "rstl/auto_ptr.hpp"

#if RSTL_KYOTO_ZIP_TYPES_DECLARED
#include "Kyoto/Streams/ZipTypes.hpp"
#else
#include <zlib.h>
#endif

class CZipOutputStream : public COutputStream {
public:
  CZipOutputStream(COutputStream* out, int level);
  ~CZipOutputStream();

  void Finish();
  bool Process(bool finish);
  void Write(const void* ptr, size_t len);

#if RSTL_KYOTO_ZIP_OUTPUT_FINISH
  int GetCompressedBytesWritten();
#else
  int GetCompressedBytesWritten() { return mCompressedBytesWritten; }
#endif

private:
  COutputStream* mOutput;
  int mCompressedBytesWritten;
  rstl::auto_ptr< z_stream > mZStream;
#if RSTL_KYOTO_ZIP_OUTPUT_FINISH
  bool mFinished;
#else
  int mUnk;
#endif
};

#endif // _CZIPOUTPUTSTREAM
