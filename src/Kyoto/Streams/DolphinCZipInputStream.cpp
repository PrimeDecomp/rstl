#include "Kyoto/Streams/CZipInputStream.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Streams/CZipSupport.hpp"

static const CInputStream::SBufferAndSize
get_buffer_and_size(CInputStream& in, unsigned int compressedLen, unsigned int decompressedLen) {
  rstl::auto_ptr< uchar > buffer(static_cast< uchar* >(CMemory::Alloc(
      decompressedLen, IAllocator::kHI_RoundUpLen, IAllocator::kSC_Unk1, IAllocator::kTP_Heap,
      CCallStack(-1, "DolphinCZipInputStream.cpp(11) : ", kUnknownType))));
  CZipSupport::Decompress(in.Get(compressedLen), compressedLen, buffer.get(), decompressedLen);
  return CInputStream::SBufferAndSize(buffer.release(), decompressedLen);
}

CZipInputStream::CZipInputStream(rstl::auto_ptr< CInputStream >& input,
                                 unsigned int compressedLength, unsigned int uncompressedLength)
: CInputStream(get_buffer_and_size(*input, compressedLength, uncompressedLength), true) {
  rstl::auto_ptr< CInputStream > stream(input);
}

CZipInputStream::~CZipInputStream() {}
