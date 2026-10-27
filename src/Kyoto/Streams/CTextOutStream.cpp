#include "Kyoto/Streams/CTextOutStream.hpp"

#include "Kyoto/Streams/COutputStream.hpp"

#include <stdarg.h>
#include <stdio.h>

CTextOutStream::CTextOutStream(COutputStream& output) : mOutput(output) {}

void CTextOutStream::WriteString(const rstl::string& text) {
  mOutput.DoPut(text.data(), text.size());
}

void CTextOutStream::WriteString(const char* format, ...) {
  char buffer[1024];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  WriteString(rstl::string(buffer));
}
