#ifndef _ICONNECTION
#define _ICONNECTION

// Guessed name

class IConnection {
public:
  virtual ~IConnection() = 0;
  // Guessed name
  virtual void Detach() = 0;
};

inline IConnection::~IConnection() {}

// Guessed name
template < typename Owner, typename Key >
class TConnection : public IConnection {
public:
  typedef void (Owner::*DisconnectFn)(Key);

  TConnection(Key key, Owner* owner, DisconnectFn disconnect)
  : mKey(key), mOwner(owner), mDisconnect(disconnect) {}
  ~TConnection() {
    if (mOwner != nullptr) {
      (mOwner->*mDisconnect)(mKey);
    }
  }

  void Detach() { mOwner = nullptr; }
  void SetKey(Key key) { mKey = key; }

private:
  Key mKey;
  Owner* mOwner;
  DisconnectFn mDisconnect;
};

#endif
