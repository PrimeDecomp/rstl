#ifndef _CBBASUPPORT
#define _CBBASUPPORT

#include "rstl/types.h"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CGuid.hpp"
#include "Kyoto/CAssetId.hpp"
#include "Kyoto/Network/CNetworkMessage.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/EFileError.hpp"
#include "Kyoto/TFunctor.hpp"
#include "rstl/list.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CInputStream;
class COutputStream;
// Guessed names
struct SNetworkWorld {
  SNetworkWorld(const CAssetId& id, const rstl::string& name) : mId(id), mName(name) {}

  CAssetId mId;
  rstl::string mName;
};

// Guessed name
enum EBBAOpenMode {
  kBOM_Read,
  kBOM_Append,
  kBOM_Write,
};

// Guessed name
enum EBBASeekOrigin {
  kBSO_Start,
  kBSO_Current,
  kBSO_End,
};

// Guessed names
enum EBBAStringChannel {
  kSC_Channel0,
  kSC_HostCommand,
  kSC_Channel2,
  kSC_ConnectionStatus,
  kSC_Channel4,
  kSC_Count,
};

// Guessed names
enum EBBADataChannel {
  kDC_Channel0,
  kDC_FileWrite,
  kDC_Channel2,
  kDC_Count,
};

// Guessed name
struct SNamedData {
  SNamedData(const rstl::string& name, SOwnedBuffer data, int size);
  explicit SNamedData(CInputStream& in);
  void Write(COutputStream& out) const;

  rstl::string mName;
  int mSize;
  SOwnedBuffer mData;
};

// Guessed name
struct SStringMessage {
  explicit SStringMessage(const rstl::string& text);
  explicit SStringMessage(CInputStream& in);

  rstl::string mText;
};

// Guessed name
typedef rstl::rc_ptr< SNetworkMessageBody > SReceivedMessage;

// Guessed name
struct SOpenFile {
  SOpenFile(const rstl::string& filename, int size, EBBAOpenMode mode);

  rstl::string mFilename;
  uint mSize;
  EBBAOpenMode mMode;
  uint mPosition;
};

// Guessed name
struct SFilenameResult {
  SFilenameResult(const rstl::string& filename, int size, FourCC type);
  ~SFilenameResult();

  rstl::string mFilename;
  int mSize;
  FourCC mType;
};

class CBBASupport {
public:
  enum EBBAErrors {
    kFE_OK,
    kFE_CantInitializeBBA,
    kFE_TimedOut,
    kFE_InvalidOpenMode,
    kFE_CantOpen,
    kFE_NotOpen,
    kFE_CantClose,
    kFE_CouldntRead,
    kFE_PartialRead,
    kFE_CouldntWrite,
    kFE_PartialWrite,
    kFE_CouldntSeek,
    kFE_InvalidSeekType,
  };

  // Guessed names
  typedef rstl::list< SReceivedMessage > SReceivedMessageList;
  typedef SReceivedMessageList::node SReceivedMessageNode;
  // Guessed names
  typedef rstl::list< SOpenFile > SOpenFileList;
  typedef SOpenFileList::node SOpenFileNode;

  static void Printf(const char* format, ...);

  static bool SendString(const rstl::string& text, int channel, const CGuid* guid);

  // Guessed name
  static bool IsConnectionActive();
  // Guessed name
  static bool IsBBAFullyConnected();
  // Guessed name
  static bool WaitForConnection();
  // Guessed name
  static SNetworkWorld GetNetworkWorld();
  // Guessed name
  static CAssetId GetAutoStartWorld();
  // Guessed name
  static SFilenameResult GetFilenameForAssetId(CAssetId id);

  static int BBAOpen(const char* path, EBBAOpenMode mode, int* handle);
  static int BBAWrite(int handle, const void* data, int size);
  // Guessed names
  static int BBAGetFileSize(int handle, int* size);
  static int BBARead(int handle, void* buffer, int size);
  static int BBASeek(int handle, int offset, EBBASeekOrigin whence);
  static int BBAGetBytesRemaining(int handle, int* remaining);
  // Guessed name
  static int DeleteFile(const char* name);
  static int BBAClose(int handle);

  // Guessed names
  struct SRemoteCommandResult {
    SRemoteCommandResult(const rstl::string& output, int status)
    : mOutput(output), mStatus(status) {}

    rstl::string mOutput;
    int mStatus;
  };
  // Guessed name
  static SRemoteCommandResult ExecuteRemoteSystemCommand(const rstl::string& command,
                                                         float timeout);
  // Guessed name
  static rstl::string GetRemoteEnvironmentVariable(const rstl::string& name);

  static int InitializeBBA(int);
  static void RegisterStringMessageCallback(int,
                                            const TFunctor2< int, const rstl::string& >& callback);
  static void Shutdown();
  static int PollMessages();

  static void RefreshNetworkAssets();
  // Guessed names
  struct SNetworkAsset {
    SNetworkAsset(const rstl::string& filename, CAssetId id, FourCC type)
    : mFilename(filename), mTag(type, id) {}

    bool operator<(const SNetworkAsset& other) const { return mTag.GetId() < other.mTag.GetId(); }

    rstl::string mFilename;
    SObjectTag mTag;
  };
  static const rstl::vector< SNetworkAsset >& GetNetworkAssets();
  // Guessed name
  static const SNetworkAsset& FindNetworkAsset(CAssetId id);

private:
  // Guessed name
  static rstl::vector< SNetworkAsset > ParseNetworkAssets(const rstl::string& text);
  // Guessed name
  static rstl::optional_object< SReceivedMessageList::iterator >
  WaitForMessageWithGuid(const CGuid& guid, float timeout);
  static bool SendNamedDataChunk(const rstl::string& name, SOwnedBuffer data, int size, int channel,
                                 CGuid guid);
  // Guessed name
  static bool SendNamedDataMessage(const rstl::string& name, SOwnedBuffer data, int size,
                                   int channel, const CGuid* guid);
  // Guessed name
  static int SendMessagePackets(const SNetworkMessageBody& message);
  // Guessed name
  static bool SendStringMessage(const rstl::string& text, int channel, const CGuid* guid);
  // Guessed name
  static SOpenFileList::iterator FindOpenFile(int handle);
  // Guessed name
  static SFilenameResult ConstructInvalidFilenameResult();
  // Guessed name
  static SReceivedMessageList::iterator
  RemoveReceivedMessage(const SReceivedMessageList::iterator& message);
  // Guessed name
  static int ReceiveSocketBytes(void* data, int length);
  // Guessed name
  static int SendSocketBytes(const void* data, int length);
  // Guessed name
  static bool HasPendingSocketBytes();
  // Guessed name
  static uint CalculatePacketCRC(const void* data, uint length);
  // Guessed name
  static void HandleAsyncConnectionCompletion(int channel, const rstl::string& message);
  // Guessed name
  static int ReceiveMessagePackets();
  // Guessed names
  static rstl::string DecodeStringMessage(const SNetworkMessageBody* message);
  static SNamedData DecodeNamedDataMessage(const SNetworkMessageBody* message);
  // Guessed name
  static void DispatchReceivedMessages();
  // Guessed name
  static int PollSocketAndSendHeartbeat();
};

// Guessed name
#define RS_DEBUGGER_BBA_PRINT(...) (rs_debugger_printf(__VA_ARGS__), CBBASupport::Printf(__VA_ARGS__))

#endif // _CBBASUPPORT
