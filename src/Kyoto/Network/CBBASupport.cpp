#include "Kyoto/Network/CBBASupport.hpp"

#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Basics/CStopwatch.hpp"
#include "Kyoto/CCrc32.hpp"
#include "Kyoto/Network/DolphinCBBACommon.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"
#include "Kyoto/Streams/COutputStreamVector.hpp"
#include "Kyoto/Text/CStringTokenizer.hpp"
#include "dolphin/os.h"
#include "rstl/StringExtras.hpp"
#include "rstl/algorithm.hpp"

#include <alloca.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
// Guessed names
int fn_8042257C(int socket, void* buffer, int length, int flags);
int fn_804227D8(int socket, const void* buffer, int length, int flags);
void fn_80422394(int socket);
int fn_80422554(int socket, int* value, int flags);
int fn_80421864(int family, int type, int protocol);
int fn_804223C8(int socket, const void* address);
int fn_80423134(int socket, int level, int option, const void* value, int length);
int fn_80423168(int socket, int command, ...);
rstl::string fn_80505588(const rstl::string& text);
}

// Guessed names
static const uint kMessageHeaderMagic = 'MHDR';
static const uint kPacketHeaderMagic = 'PHDR';
static const int kPacketPayloadMax = 0x1000;
// Guessed name
static const int kStreamBufferLength = 0x1000;

struct SMessageHeader {
  uint mMagic;
  uint mLength;
};

struct SPacketHeader {
  uint mMagic;
  uint mIndex;
  uint mCrc;
  uint mLength;
};

// Guessed names
struct SBBASocketAddress {
  uchar mLength;
  uchar mFamily;
  ushort mPort;
  uint mAddress;
};

// Guessed name
static inline void SwapWord(uint& word) {
  uchar* bytes = reinterpret_cast< uchar* >(&word);
  uchar temp = bytes[0];
  bytes[0] = bytes[3];
  bytes[3] = temp;
  temp = bytes[1];
  bytes[1] = bytes[2];
  bytes[2] = temp;
}

// Guessed names
static CGuid sLocalGuid = CGuid::sDefault;
static int sSocket = -1;
static bool sInitialized;
static bool sConnectionActive;
static bool sAsyncConnectionPending;
static CBBASupport::SReceivedMessageList sReceivedMessages[kMT_Count];
static TFunctor2< int, const rstl::string& > sStringCallbacks[kSC_Count];
static TFunctor2< int, const SNamedData& > sDataCallbacks[kDC_Count];
static CBBASupport::SOpenFileList sOpenFiles;
static rstl::vector< CBBASupport::SNetworkAsset > sNetworkAssets;

SFilenameResult::~SFilenameResult() {}

void CBBASupport::Printf(const char* format, ...) {
  char buffer[2048];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  if (IsConnectionActive()) {
    SendString(rstl::string_l("Print ") + rstl::string(buffer), kSC_Channel0, nullptr);
  } else {
    rs_debugger_printf("rs_bba_printf: ");
    rs_debugger_printf("%s", buffer);
  }
  va_end(args);
}

SFilenameResult::SFilenameResult(const rstl::string& filename, int size, FourCC type)
: mFilename(filename), mSize(size), mType(type) {}

SFilenameResult CBBASupport::ConstructInvalidFilenameResult() {
  return SFilenameResult(rstl::string(), -1, -1);
}

uint CBBASupport::CalculatePacketCRC(const void* data, uint length) {
  return CCRC32::Calculate(data, length, ~0u);
}

int CBBASupport::PollSocketAndSendHeartbeat() {
  static CStopwatch heartbeatTimer;
  int value;
  const int result = fn_80422554(sSocket, &value, 0);
  if (heartbeatTimer.GetElapsedTime() > 0.25f) {
    heartbeatTimer.Reset();
    SendStringMessage(rstl::string("game_ping_connection"), kSC_HostCommand, nullptr);
  }
  return result;
}

bool CBBASupport::IsBBAFullyConnected() {
  if (!sConnectionActive) {
    return false;
  }
  if (!sAsyncConnectionPending) {
    return true;
  }
  const int status = PollSocketAndSendHeartbeat();
  if (status == -56) {
    OSReport("************************************************************************\n");
    OSReport("Couldn't connect socket in CBBASupport::IsBBAFullyConnected. Error(%d).\n", status);
    OSReport("************************************************************************\n");
    fn_80422394(sSocket);
    sConnectionActive = false;
    sAsyncConnectionPending = false;
  }
  return false;
}

int CBBASupport::InitializeBBA(int mode) {
  if (!sInitialized) {
    if (!BBA_InitializeHardwareFromSettings()) {
      return kFE_CantInitializeBBA;
    }
    sInitialized = true;
  }
  if (!sConnectionActive) {
    sLocalGuid = CGuid::Generate();
    CStopwatch timer;
    sSocket = fn_80421864(2, 1, 0);
    if (sSocket < 0) {
      return kFE_CantInitializeBBA;
    }
    if (mode != 1) {
      const int flags = fn_80423168(sSocket, 3, 0);
      fn_80423168(sSocket, 4, flags | 4);
      sAsyncConnectionPending = true;
    }
    SBBASocketAddress address;
    memset(&address, 0, sizeof(address));
    address.mLength = sizeof(address);
    address.mFamily = 2;
    address.mPort = BBA_GetServerPort();
    address.mAddress = BBA_GetServerIPv4Address();
    const int excess = reinterpret_cast< uint >(&address) -
                       reinterpret_cast< uint >(OSGetCurrentThread()->stackEnd) - 0x4000;
    alloca(excess < 0 ? 0 : excess);
    const int result = fn_804223C8(sSocket, &address);
    if (result != 0 && result != -26) {
      OSReport("*****************************************************************\n");
      OSReport("Couldn't connect socket in CBBASupport::InitializeBBA. Error(%d).\n", result);
      OSReport("*****************************************************************\n");
      fn_80422394(sSocket);
      return kFE_CantInitializeBBA;
    }
    const int flags = fn_80423168(sSocket, 3, 0);
    fn_80423168(sSocket, 4, flags & ~4);
    int enabled = 1;
    fn_80423134(sSocket, 0xffff, 4, &enabled, sizeof(enabled));
    OSReport(CBasics::Stringize("BBA connection time: %.1fs\n", timer.GetElapsedTime()));
    sStringCallbacks[kSC_ConnectionStatus] =
        TStaticCallback2< int, const rstl::string& >::Make(HandleAsyncConnectionCompletion);
    sConnectionActive = true;
  }
  return kFE_OK;
}

void CBBASupport::Shutdown() {
  if (!sInitialized) {
    return;
  }
  if (sConnectionActive) {
    OSReport("BBA Socket shutdown.\n");
    sConnectionActive = false;
  }
  BBA_ShutdownHardware();
  for (int channel = 0; channel < kMT_Count; ++channel) {
    sReceivedMessages[channel] = SReceivedMessageList();
  }
  sNetworkAssets = rstl::vector< SNetworkAsset >();
  sInitialized = false;
}

void CBBASupport::RegisterStringMessageCallback(
    int channel, const TFunctor2< int, const rstl::string& >& callback) {
  sStringCallbacks[channel] = callback;
}

bool CBBASupport::WaitForConnection() {
  while (!IsBBAFullyConnected()) {
    if (!sConnectionActive) {
      return false;
    }
    CStopwatch::Wait(0.5f);
    rs_debugger_printf("Waiting for BBA socket connection..\n");
    PollMessages();
  }
  return true;
}

void CBBASupport::HandleAsyncConnectionCompletion(int, const rstl::string& message) {
  if (message == rstl::string_l("game_connection_received") && sAsyncConnectionPending) {
    rs_debugger_printf("\n*** Async BBA connection completed OK! ***\n");
    sAsyncConnectionPending = false;
  }
}

CBBASupport::SReceivedMessageList::iterator
CBBASupport::RemoveReceivedMessage(const SReceivedMessageList::iterator& message) {
  for (int channel = 0; channel < kMT_Count; ++channel) {
    SReceivedMessageList& queue = sReceivedMessages[channel];
    for (SReceivedMessageList::iterator it = queue.begin(); it != queue.end(); ++it) {
      if (it == message) {
        return queue.erase(it);
      }
    }
  }
  return message;
}

void CBBASupport::DispatchReceivedMessages() {
  for (int channel = 0; channel < kMT_Count; ++channel) {
    SReceivedMessageList& queue = sReceivedMessages[channel];
    for (SReceivedMessageList::iterator it = queue.begin(); it != queue.end(); ++it) {
      const SNetworkMessageBody* body = (*it).GetPtr();
      const EBBAMessageType type = body->mType;
      int index;
      switch (type) {
      case kMT_String0:
      case kMT_String1:
      case kMT_HostCommand:
      case kMT_ConnectionStatus:
      case kMT_String4: {
        if (type == kMT_String0) {
          index = kSC_Channel2;
        } else if (type == kMT_String1) {
          index = kSC_Channel0;
        } else if (type == kMT_HostCommand) {
          index = kSC_HostCommand;
        } else {
          index = kSC_Channel4;
          if (type == kMT_ConnectionStatus) {
            index = kSC_ConnectionStatus;
          }
        }
        const TFunctor2< int, const rstl::string& >& callback = sStringCallbacks[index];
        if (callback) {
          callback(index, DecodeStringMessage(body));
        }
        break;
      }
      case kMT_NamedData0:
      case kMT_NamedData1:
      case kMT_NamedData2: {
        if (type == kMT_NamedData0) {
          index = kDC_Channel0;
        } else {
          index = kDC_Channel2;
          if (type == kMT_NamedData1) {
            index = kDC_FileWrite;
          }
        }
        const TFunctor2< int, const SNamedData& >& callback = sDataCallbacks[index];
        if (callback) {
          callback(index, DecodeNamedDataMessage(body));
        }
        break;
      }
      default:
        RS_VERIFY_FAILURE(584, "false", "false",
                          CBasics::Stringize("Unhandled user type %d", type));
        break;
      }
      it = RemoveReceivedMessage(it);
    }
  }
}

int CBBASupport::PollMessages() {
  const int received = ReceiveMessagePackets();
  DispatchReceivedMessages();
  return received;
}

bool CBBASupport::HasPendingSocketBytes() {
  char peek[8];
  return fn_8042257C(sSocket, peek, sizeof(peek), 6) > 0;
}

int CBBASupport::ReceiveSocketBytes(void* data, const int length) {
  uchar* cursor = static_cast< uchar* >(data);
  int remaining = length;
  while (remaining > 0) {
    const int received = fn_8042257C(sSocket, cursor, remaining, 0);
    if (received < -1) {
      rs_debugger_printf("error %d during receive...\n", received);
      return -1;
    }
    remaining -= received;
    cursor += received;
  }
  return length;
}

int CBBASupport::SendSocketBytes(const void* data, int length) {
  const int sent = fn_804227D8(sSocket, data, length, 0);
  if (static_cast< uint >(sent) == static_cast< uint >(length)) {
    return length;
  }
  if (sent < 0) {
    rs_debugger_printf("error %d delivering message...\n", sent);
    return -1;
  }
  return -1;
}

bool CBBASupport::IsConnectionActive() { return sConnectionActive; }

CAssetId CBBASupport::GetAutoStartWorld() {
  CAssetId world = kInvalidAssetId;
  if (!IsConnectionActive()) {
    return world;
  }
  if (!IsBBAFullyConnected()) {
    return world;
  }
  rstl::string command("getautostartworld");
  CGuid guid = CGuid::Generate();
  SendString(command, kSC_HostCommand, &guid);
  rstl::optional_object< SReceivedMessageList::iterator > reply =
      WaitForMessageWithGuid(guid, 0.2f);
  if (!reply) {
    return world;
  }
  rstl::string text = DecodeStringMessage((**reply).GetPtr());
  CStringTokenizer tokenizer(text.c_str());
  rstl::string prefix = tokenizer.ReadToken(0, '"');
  if (prefix != rstl::string_l("autostartworld")) {
    return world;
  }
  rstl::string id = tokenizer.ReadToken(0, '"');
  world = CAssetId(id.c_str());
  const SReceivedMessageList::iterator replyIterator = *reply;
  RemoveReceivedMessage(replyIterator);
  return world;
}

SNetworkWorld CBBASupport::GetNetworkWorld() {
  SNetworkWorld world(kInvalidAssetId, rstl::string_l(""));
  if (!IsConnectionActive()) {
    return world;
  }
  if (!IsBBAFullyConnected()) {
    return world;
  }
  rstl::string command("getnetworkworld");
  CGuid guid = CGuid::Generate();
  SendString(command, kSC_HostCommand, &guid);
  rstl::optional_object< SReceivedMessageList::iterator > reply =
      WaitForMessageWithGuid(guid, 0.2f);
  if (!reply) {
    return world;
  }
  rstl::string text = DecodeStringMessage((**reply).GetPtr());
  CStringTokenizer tokenizer(text.c_str());
  rstl::string prefix = tokenizer.ReadToken(0, '"');
  if (prefix != rstl::string_l("networkworld")) {
    return world;
  }
  world.mName = tokenizer.ReadToken(0, '"');
  rstl::string id = tokenizer.ReadToken(0, '"');
  world.mId = CAssetId(id.c_str());
  const SReceivedMessageList::iterator replyIterator = *reply;
  RemoveReceivedMessage(replyIterator);
  return world;
}

CBBASupport::SRemoteCommandResult
CBBASupport::ExecuteRemoteSystemCommand(const rstl::string& command, float timeout) {
  SRemoteCommandResult failure(rstl::string_l(""), -1);
  if (!IsConnectionActive()) {
    return failure;
  }
  if (!IsBBAFullyConnected()) {
    return failure;
  }
  rstl::string request("system ");
  request.append(command);
  CGuid guid = CGuid::Generate();
  SendString(request, kSC_HostCommand, &guid);
  rstl::optional_object< SReceivedMessageList::iterator > reply =
      WaitForMessageWithGuid(guid, timeout);
  if (!reply) {
    return failure;
  }
  SReceivedMessage message = **reply;
  const SReceivedMessageList::iterator replyIterator = *reply;
  RemoveReceivedMessage(replyIterator);
  rstl::string text = DecodeStringMessage(message.GetPtr());
  CStringTokenizer tokenizer(text.c_str());
  rstl::string prefix("systemresult");
  if (CStringExtras::CompareCaseInsensitive(tokenizer.ReadToken(0, '"'), prefix) != 0) {
    return failure;
  }
  const int status = atoi(tokenizer.ReadToken(0, '"').c_str());
  return SRemoteCommandResult(rstl::string(tokenizer.GetRemainingText()), status);
}

rstl::string CBBASupport::GetRemoteEnvironmentVariable(const rstl::string& name) {
  rstl::string empty("");
  if (!IsConnectionActive()) {
    return empty;
  }
  if (!IsBBAFullyConnected()) {
    return empty;
  }
  rstl::string command("getenv ");
  command.append(name);
  CGuid guid = CGuid::Generate();
  SendString(command, kSC_HostCommand, &guid);
  rstl::optional_object< SReceivedMessageList::iterator > reply =
      WaitForMessageWithGuid(guid, 1.4f);
  if (!reply) {
    return empty;
  }
  SReceivedMessage message = **reply;
  const SReceivedMessageList::iterator replyIterator = *reply;
  RemoveReceivedMessage(replyIterator);
  rstl::string text = DecodeStringMessage(message.GetPtr());
  rstl::string prefix("envvar");
  if (CStringExtras::CompareCaseInsensitive(text.substr(0, prefix.length()), prefix) != 0) {
    return empty;
  }
  text = text.substr(prefix.length());
  return fn_80505588(text);
}

const rstl::vector< CBBASupport::SNetworkAsset >& CBBASupport::GetNetworkAssets() {
  return sNetworkAssets;
}

const CBBASupport::SNetworkAsset& CBBASupport::FindNetworkAsset(CAssetId id) {
  const SNetworkAsset key(rstl::string(), id, 0);
  rstl::vector< SNetworkAsset >::iterator it =
      rstl::lower_bound(sNetworkAssets.begin(), sNetworkAssets.end(), key);
  if (it == sNetworkAssets.end() || it->mTag.GetId() != id) {
    static const SNetworkAsset kEmptyAsset(rstl::string_l("kEmptyAsset"), kInvalidAssetId, -1);
    return kEmptyAsset;
  }
  return *it;
}

rstl::vector< CBBASupport::SNetworkAsset >
CBBASupport::ParseNetworkAssets(const rstl::string& text) {
  CStringTokenizer tokenizer(text.c_str());
  rstl::vector< SNetworkAsset > assets;
  rstl::string filename = tokenizer.ReadToken(0, '"');
  CAssetId id(tokenizer.ReadToken(0, '"').c_str());
  FourCC type = atoi(tokenizer.ReadToken(0, '"').c_str());
  while (filename.length() != 0) {
    if (assets.capacity() <= assets.size()) {
      assets.reserve(assets.capacity() + 2);
    }
    assets.push_back(SNetworkAsset(filename, id, type));
    filename = tokenizer.ReadToken(0, '"');
    id = CAssetId(tokenizer.ReadToken(0, '"').c_str());
    type = atoi(tokenizer.ReadToken(0, '"').c_str());
  }
  rstl::sort(assets.begin(), assets.end());
  return assets;
}

void CBBASupport::RefreshNetworkAssets() {
  if (!IsConnectionActive() || !IsBBAFullyConnected()) {
    return;
  }
  rstl::string command("getnetworkassets");
  CGuid guid = CGuid::Generate();
  SendString(command, kSC_HostCommand, &guid);
  rstl::optional_object< SReceivedMessageList::iterator > reply =
      WaitForMessageWithGuid(guid, 1.4f);
  if (!reply) {
    return;
  }
  SReceivedMessage message = **reply;
  const SReceivedMessageList::iterator replyIterator = *reply;
  RemoveReceivedMessage(replyIterator);
  rstl::string text = DecodeStringMessage(message.GetPtr());
  const rstl::string prefix("NETWORKASSETS");
  if (CStringExtras::CompareCaseInsensitive(text.substr(0, prefix.length()), prefix) != 0) {
    return;
  }
  text = text.substr(prefix.length());
  sNetworkAssets = ParseNetworkAssets(text);
}

CBBASupport::SOpenFileList::iterator CBBASupport::FindOpenFile(int handle) {
  if (handle == -1) {
    return sOpenFiles.end();
  }
  for (SOpenFileList::iterator it = sOpenFiles.begin(); it != sOpenFiles.end(); ++it) {
    if (handle == reinterpret_cast< int >(&*it)) {
      return it;
    }
  }
  return sOpenFiles.end();
}

SOpenFile::SOpenFile(const rstl::string& filename, int size, EBBAOpenMode mode)
: mFilename(filename), mSize(size), mMode(mode), mPosition(0) {}

int CBBASupport::BBAOpen(const char* path, EBBAOpenMode mode, int* handle) {
  if (!WaitForConnection()) {
    rs_debugger_printf("BBA System not initialized in CBBASupport::BBAOpen.\n");
    return kFE_CantOpen;
  }
  *handle = -1;
  rstl::string command("fileopen ");
  switch (mode) {
  case kBOM_Read:
    command.append(rstl::string_l("read"));
    break;
  case kBOM_Append:
    command.append(rstl::string_l("append"));
    break;
  case kBOM_Write:
    command.append(rstl::string_l("write"));
    break;
  }
  command.append(CBasics::Stringize(" \"%s\"", path), -1);
  CGuid guid = CGuid::Generate();
  SendString(command, kSC_HostCommand, &guid);
  rstl::optional_object< SReceivedMessageList::iterator > reply =
      WaitForMessageWithGuid(guid, 15.f);
  if (!reply) {
    return kFE_TimedOut;
  }
  SReceivedMessage message = **reply;
  const SReceivedMessageList::iterator replyIterator = *reply;
  RemoveReceivedMessage(replyIterator);
  rstl::string text = DecodeStringMessage(message.GetPtr());
  CStringTokenizer tokenizer(text.c_str());
  rstl::string status = tokenizer.ReadToken(0, '"');
  if (status != rstl::string_l("filesize")) {
    return kFE_CantOpen;
  }
  const int size = atoi(tokenizer.ReadToken(0, '"').c_str());
  if (size < 0) {
    return kFE_CantOpen;
  }
  SOpenFile file(rstl::string(path, -1), size, mode);
  SOpenFileList::iterator opened = sOpenFiles.insert(sOpenFiles.begin(), file);
  *handle = reinterpret_cast< int >(&*opened);
  return kFE_OK;
}

int CBBASupport::BBAClose(int handle) {
  if (!WaitForConnection()) {
    rs_debugger_printf("BBA System not initialized in CBBASupport::BBAClose.\n");
    return kFE_CantClose;
  }
  const SOpenFileList::iterator file = FindOpenFile(handle);
  if (file == sOpenFiles.end()) {
    return kFE_CantClose;
  }
  sOpenFiles.erase(file);
  return kFE_OK;
}

int CBBASupport::BBARead(int handle, void* buffer, int size) {
  if (!WaitForConnection()) {
    rs_debugger_printf("BBA System not initialized in CBBASupport::BBARead.\n");
    return kFE_CouldntRead;
  }
  const SOpenFileList::iterator file = FindOpenFile(handle);
  if (file == sOpenFiles.end()) {
    return kFE_NotOpen;
  }
  if (file->mMode != kBOM_Read) {
    return kFE_CouldntRead;
  }
  if (size + file->mPosition > file->mSize) {
    return kFE_CouldntRead;
  }
  rstl::string command(
      CBasics::Stringize("fileread \"%s\" %d %d", file->mFilename.c_str(), file->mPosition, size),
      -1);
  CGuid guid = CGuid::Generate();
  SendString(command, kSC_HostCommand, &guid);
  rstl::optional_object< SReceivedMessageList::iterator > reply =
      WaitForMessageWithGuid(guid, 15.f);
  if (!reply) {
    return kFE_TimedOut;
  }
  SReceivedMessage message = **reply;
  const SReceivedMessageList::iterator replyIterator = *reply;
  RemoveReceivedMessage(replyIterator);
  SNamedData data = DecodeNamedDataMessage(message.GetPtr());
  if (data.mName != rstl::string_l("FileReadOK")) {
    return kFE_CouldntRead;
  }
  memcpy(buffer, data.mData.mData, data.mSize);
  DCStoreRange(buffer, data.mSize);
  file->mPosition += size;
  return kFE_OK;
}

int CBBASupport::BBAWrite(int handle, const void* data, int size) {
  if (!WaitForConnection()) {
    rs_debugger_printf("BBA System not initialized in CBBASupport::BBAWrite.\n");
    return kFE_CouldntWrite;
  }
  const SOpenFileList::iterator file = FindOpenFile(handle);
  if (file == sOpenFiles.end()) {
    return kFE_NotOpen;
  }
  if (file->mMode == kBOM_Read) {
    return kFE_CouldntWrite;
  }
  rstl::string command(CBasics::Stringize("filewrite \"%s\"", file->mFilename.c_str()), -1);
  SOwnedBuffer buffer(data, false);
  if (!SendNamedDataChunk(command, buffer, size, kDC_FileWrite, CGuid::Generate())) {
    return kFE_CouldntWrite;
  }
  return kFE_OK;
}

int CBBASupport::BBASeek(int handle, int offset, EBBASeekOrigin whence) {
  if (!WaitForConnection()) {
    rs_debugger_printf("BBA System not initialized in CBBASupport::BBASeek.\n");
    return kFE_CouldntSeek;
  }
  const SOpenFileList::iterator file = FindOpenFile(handle);
  if (file == sOpenFiles.end()) {
    return kFE_NotOpen;
  }
  uint position = file->mPosition;
  switch (whence) {
  case kBSO_Start:
    position = offset;
    break;
  case kBSO_Current:
    position += offset;
    break;
  case kBSO_End:
    position = offset + file->mSize;
    break;
  default:
    return kFE_InvalidSeekType;
  }
  if (position > file->mSize) {
    return kFE_CouldntSeek;
  }
  file->mPosition = position;
  return kFE_OK;
}

int CBBASupport::BBAGetFileSize(int handle, int* size) {
  if (!WaitForConnection()) {
    rs_debugger_printf("BBA System not initialized in CBBASupport::BBAGetFileSize.\n");
    return kFE_NotOpen;
  }
  const SOpenFileList::iterator file = FindOpenFile(handle);
  if (file == sOpenFiles.end()) {
    return kFE_NotOpen;
  }
  *size = file->mSize;
  return kFE_OK;
}

int CBBASupport::DeleteFile(const char* name) {
  if (!WaitForConnection()) {
    rs_debugger_printf("BBA System not initialized in CBBASupport::BBAOpen.\n");
    return kFE_CantOpen;
  }
  rstl::string command(CBasics::Stringize("filedelete \"%s\"", name), -1);
  CGuid guid = CGuid::Generate();
  SendString(command, kSC_HostCommand, &guid);
  rstl::optional_object< SReceivedMessageList::iterator > reply =
      WaitForMessageWithGuid(guid, 15.f);
  if (!reply) {
    return kFE_TimedOut;
  }
  SReceivedMessage message = **reply;
  const SReceivedMessageList::iterator replyIterator = *reply;
  RemoveReceivedMessage(replyIterator);
  rstl::string text = DecodeStringMessage(message.GetPtr());
  CStringTokenizer tokenizer(text.c_str());
  rstl::string status = tokenizer.ReadToken(0, '"');
  if (status != rstl::string_l("DeleteSuccessful")) {
    return kFE_CantOpen;
  }
  return kFE_OK;
}

int CBBASupport::BBAGetBytesRemaining(int handle, int* remaining) {
  if (!WaitForConnection()) {
    rs_debugger_printf("BBA System not initialized in CBBASupport::BBAGetBytesRemaining.\n");
    return kFE_NotOpen;
  }
  const SOpenFileList::iterator file = FindOpenFile(handle);
  if (file == sOpenFiles.end()) {
    return kFE_NotOpen;
  }
  *remaining = file->mSize - file->mPosition;
  return kFE_OK;
}

bool CBBASupport::SendString(const rstl::string& text, int channel, const CGuid* guid) {
  if (!WaitForConnection()) {
    rs_debugger_printf("BBA system not initialized in CBBASupport::SendString.\n");
    return false;
  }
  return SendStringMessage(text, channel, guid);
}

bool CBBASupport::SendNamedDataChunk(const rstl::string& name, SOwnedBuffer data, int size,
                                     int channel, CGuid guid) {
  if (!WaitForConnection()) {
    rs_debugger_printf("BBA system not initialized in CBBASupport::SendNamedDataChunk.\n");
    return false;
  }
  return SendNamedDataMessage(name, data, size, channel, &guid);
}

bool CBBASupport::SendNamedDataMessage(const rstl::string& name, SOwnedBuffer data, int size,
                                       int channel, const CGuid* guid) {
  const EBBAMessageType types[kDC_Count] = {kMT_NamedData0, kMT_NamedData1, kMT_NamedData2};
  const EBBAMessageType type = types[channel];
  SNamedData named(name, data, size);
  COutputStreamVector stream(kStreamBufferLength);
  named.Write(stream);
  const int length = stream.GetBuffer().size();
  SOwnedBuffer payload(rs_new_line(747) uchar[length], true);
  memcpy(payload.mData, stream.GetBuffer().data(), length);
  SNetworkMessageBody message(sLocalGuid, guid != nullptr ? *guid : CGuid::Generate(), type, length,
                              payload);
  return SendMessagePackets(message) >= 0;
}

bool CBBASupport::SendStringMessage(const rstl::string& text, int channel, const CGuid* guid) {
  const EBBAMessageType types[kSC_Count] = {kMT_String1, kMT_HostCommand, kMT_String0,
                                            kMT_ConnectionStatus, kMT_String4};
  const EBBAMessageType type = types[channel];
  SStringMessage stringMessage(text);
  COutputStreamVector stream(kStreamBufferLength);
  text.PutTo(stream);
  const int length = stream.GetBuffer().size();
  SOwnedBuffer payload(rs_new_line(707) uchar[length], true);
  memcpy(payload.mData, stream.GetBuffer().data(), length);
  SNetworkMessageBody message(sLocalGuid, guid != nullptr ? *guid : CGuid::Generate(), type, length,
                              payload);
  return SendMessagePackets(message) >= 0;
}

int CBBASupport::SendMessagePackets(const SNetworkMessageBody& message) {
  COutputStreamVector body(kStreamBufferLength);
  message.Write(body);
  const int total = body.GetWrittenBytes();
  const uchar* cursor = body.GetBuffer().data();

  COutputStreamVector wire(kStreamBufferLength);
  SMessageHeader messageHeader;
  messageHeader.mMagic = kMessageHeaderMagic;
  messageHeader.mLength = total;
  SwapWord(messageHeader.mMagic);
  SwapWord(messageHeader.mLength);
  wire.Put(&messageHeader, sizeof(messageHeader));

  int remaining = total;
  int sentBytes = 0;
  for (uint index = 0; remaining > 0; ++index) {
    const int length = remaining < kPacketPayloadMax ? remaining : kPacketPayloadMax;
    SPacketHeader packetHeader;
    packetHeader.mMagic = kPacketHeaderMagic;
    packetHeader.mIndex = index;
    packetHeader.mCrc = CalculatePacketCRC(cursor, length);
    packetHeader.mLength = length;
    SwapWord(packetHeader.mMagic);
    SwapWord(packetHeader.mIndex);
    SwapWord(packetHeader.mCrc);
    SwapWord(packetHeader.mLength);
    wire.Put(&packetHeader, sizeof(packetHeader));
    wire.Put(cursor, length);
    wire.Flush();
    const int sent =
        SendSocketBytes(wire.GetBuffer().data() + sentBytes, wire.GetBuffer().size() - sentBytes);
    sentBytes = wire.GetBuffer().size();
    if (sent < 0) {
      return sent;
    }
    remaining -= length;
    cursor += length;
  }
  return total;
}

int CBBASupport::ReceiveMessagePackets() {
  int received = 0;
  while (HasPendingSocketBytes()) {
    uchar header[sizeof(SMessageHeader) + sizeof(SPacketHeader)];
    int result = ReceiveSocketBytes(header, sizeof(header));
    if (result < 0) {
      return result;
    }
    SMessageHeader messageHeader;
    SPacketHeader packetHeader;
    memcpy(&messageHeader, header, sizeof(messageHeader));
    memcpy(&packetHeader, header + sizeof(messageHeader), sizeof(packetHeader));
    SwapWord(messageHeader.mMagic);
    SwapWord(messageHeader.mLength);
    if (messageHeader.mMagic == kMessageHeaderMagic) {
      SOwnedBuffer buffer(rs_new_line(427) uchar[messageHeader.mLength], true);
      int remaining = messageHeader.mLength;
      uchar* cursor = static_cast< uchar* >(buffer.mData);
      uint index = 0;
      while (remaining > 0) {
        SwapWord(packetHeader.mMagic);
        SwapWord(packetHeader.mIndex);
        SwapWord(packetHeader.mCrc);
        SwapWord(packetHeader.mLength);
        if (packetHeader.mMagic != kPacketHeaderMagic) {
          rs_debugger_printf("msg packet invalid header\n");
          return -1;
        }
        if (packetHeader.mIndex != index) {
          rs_debugger_printf("msg packet out of order, expected %d got %d...\n", index,
                             packetHeader.mIndex);
          return -1;
        }
        result = ReceiveSocketBytes(cursor, packetHeader.mLength);
        if (result < 0) {
          return result;
        }
        if (packetHeader.mCrc != CalculatePacketCRC(cursor, packetHeader.mLength)) {
          rs_debugger_printf("crc failure in msg packet\n");
          return -1;
        }
        ++index;
        remaining -= packetHeader.mLength;
        cursor += packetHeader.mLength;
        if (remaining > 0) {
          result = ReceiveSocketBytes(&packetHeader, sizeof(packetHeader));
          if (result < 0) {
            return result;
          }
        }
      }
      CMemoryInStream stream(buffer.mData, messageHeader.mLength);
      SReceivedMessage message(rs_new_line(501) SNetworkMessageBody(stream, sLocalGuid));
      sReceivedMessages[message->mType].push_back(message);
    } else {
      rs_debugger_printf("received msg with errors...\n");
    }
    ++received;
  }
  return received;
}

rstl::optional_object< CBBASupport::SReceivedMessageList::iterator >
CBBASupport::WaitForMessageWithGuid(const CGuid& guid, float timeout) {
  const CStopwatch timer;
  float lastReport = 0.f;
  while (timer.GetElapsedTime() < timeout) {
    if (timer.GetElapsedTime() - lastReport > timeout * 0.5f) {
      lastReport = timer.GetElapsedTime();
      rs_debugger_printf("Waiting for message with guid %s\n", guid.ToString().c_str());
    }
    ReceiveMessagePackets();
    for (int channel = 0; channel < kMT_Count; ++channel) {
      for (SReceivedMessageList::iterator it = sReceivedMessages[channel].begin();
           it != sReceivedMessages[channel].end(); ++it) {
        if ((*it)->mGuid == guid) {
          return rstl::optional_object< SReceivedMessageList::iterator >(it);
        }
      }
    }
  }
  rs_debugger_printf("Waiting for message with guid %s timed out @%f sec\n",
                     guid.ToString().c_str(), timer.GetElapsedTime());
  return rstl::optional_object< SReceivedMessageList::iterator >();
}

SFilenameResult CBBASupport::GetFilenameForAssetId(CAssetId id) {
  if (id == kInvalidAssetId) {
    return ConstructInvalidFilenameResult();
  }
  if (!IsConnectionActive()) {
    return ConstructInvalidFilenameResult();
  }
  if (!IsBBAFullyConnected()) {
    return ConstructInvalidFilenameResult();
  }
  rstl::string command(CBasics::Stringize("get_filename_for_assetid %s", id.ToHexString().c_str()),
                       -1);
  CGuid guid = CGuid::Generate();
  SendString(command, kSC_HostCommand, &guid);
  rstl::optional_object< SReceivedMessageList::iterator > reply =
      WaitForMessageWithGuid(guid, 0.2f);
  if (!reply) {
    return ConstructInvalidFilenameResult();
  }
  rstl::string text = DecodeStringMessage((**reply).GetPtr());
  CStringTokenizer tokenizer(text.c_str());
  rstl::string status = tokenizer.ReadToken(0, '"');
  if (status != rstl::string_l("assetid_filename")) {
    return ConstructInvalidFilenameResult();
  }
  rstl::string filename = tokenizer.ReadToken(0, '"');
  const int size = atoi(tokenizer.ReadToken(0, '"').c_str());
  const FourCC type = SObjectTag::Text2Type(tokenizer.ReadToken(0, '"').c_str());
  const SReceivedMessageList::iterator replyIterator = *reply;
  RemoveReceivedMessage(replyIterator);
  return SFilenameResult(rstl::string(filename), size, type);
}

