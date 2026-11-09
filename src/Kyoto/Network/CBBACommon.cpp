#include "Kyoto/Network/CBBASupport.hpp"

#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"

SStringMessage::SStringMessage(const rstl::string& text) : mText(text) {}

SStringMessage::SStringMessage(CInputStream& in) : mText(in) {}

SNamedData::SNamedData(const rstl::string& name, SOwnedBuffer data, int size)
: mName(name), mSize(size), mData(data) {}

SNamedData::SNamedData(CInputStream& in)
: mName(in), mSize(in.ReadInt32()), mData(rs_new_line(55) uchar[mSize], true) {
  in.ReadBytes(mData.mData, mSize);
}

void SNamedData::Write(COutputStream& out) const {
  mName.PutTo(out);
  out.WriteInt32(mSize);
  out.Put(mData.mData, mSize);
}

rstl::string CBBASupport::DecodeStringMessage(const SNetworkMessageBody* message) {
  CMemoryInStream stream(message->mPayload.mData, message->mSize);
  SStringMessage text(stream);
  return text.mText;
}

SNamedData CBBASupport::DecodeNamedDataMessage(const SNetworkMessageBody* message) {
  CMemoryInStream stream(message->mPayload.mData, message->mSize);
  SNamedData data(stream);
  return data;
}

