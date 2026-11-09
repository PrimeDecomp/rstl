#include "Kyoto/Network/CNetworkMessage.hpp"

#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"

SNetworkMessageBody::SNetworkMessageBody(const CGuid& sender, const CGuid& guid,
                                         EBBAMessageType type, int size, SOwnedBuffer payload)
: mSender(sender)
, mGuid(guid)
, mType(type)
, mSize(size)
, mPayload(payload)
, mReceiver(CGuid::sDefault) {}

SNetworkMessageBody::SNetworkMessageBody(CInputStream& in, const CGuid& receiver)
: mSender(in)
, mGuid(in)
, mType(static_cast< EBBAMessageType >(in.ReadInt32()))
, mSize(in.ReadInt32())
, mReceiver(receiver) {
  if (mSize > 0) {
    SOwnedBuffer buffer(rs_new_line(50) uchar[mSize], true);
    in.ReadBytes(buffer.mData, mSize);
    mPayload = buffer;
  }
}

void SNetworkMessageBody::Write(COutputStream& out) const {
  mSender.PutTo(out);
  mGuid.PutTo(out);
  out.WriteInt32(mType);
  out.WriteInt32(mSize);
  if (mSize > 0) {
    out.Put(mPayload.mData, mSize);
  }
}

