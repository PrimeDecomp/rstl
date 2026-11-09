#ifndef _TSIGNAL0
#define _TSIGNAL0

#include "types.h"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/IConnection.hpp"
#include "Kyoto/TFunctor.hpp"

#include "rstl/list.hpp"
#include "rstl/auto_ptr.hpp"

class TSignal0 {
public:
  struct SSlot {
    SSlot(IConnection* connection, const TFunctor0& functor)
    : mConnection(connection), mFunctor(functor) {}

    IConnection* mConnection;
    TFunctor0 mFunctor;
  };
  typedef rstl::list< SSlot >::iterator iterator;
  typedef rstl::list< SSlot >::const_iterator const_iterator;

  ~TSignal0() {
    for (iterator it = mSlots.begin(); it != mSlots.end(); ++it) {
      it->mConnection->Detach();
    }
  }

  typedef TConnection< TSignal0, iterator > Connection;

  void Disconnect(iterator it) { mSlots.erase(it); }

  // Guessed name
  rstl::auto_ptr< IConnection > Connect(TFunctor0 functor) {
    Connection* connection =
        rs_new_line_in("CSignal0.h", 52) Connection(mSlots.end(), this, &TSignal0::Disconnect);
    connection->SetKey(mSlots.insert(mSlots.end(), SSlot(connection, functor)));
    return connection;
  }
  void Emit() const;

private:
  rstl::list< SSlot > mSlots;
};

inline void TSignal0::Emit() const {
  const_iterator end = mSlots.end();
  const_iterator it = mSlots.begin();
  while (it != end) {
    const_iterator slot = it;
    ++it;
    slot->mFunctor();
  }
}

#endif // _TSIGNAL0
