#ifndef KYOTO_STREAMS_CFIOINSTREAM_HPP
#define KYOTO_STREAMS_CFIOINSTREAM_HPP

#include "Kyoto/Streams/CInputStream.hpp"

// Guessed name. Input stream that reads a host file.
class CFIOInStream : public CInputStream {
public:
  explicit CFIOInStream(const char* path);
};

#endif
