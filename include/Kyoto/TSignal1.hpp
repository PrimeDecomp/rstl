#ifndef _TSIGNAL1
#define _TSIGNAL1

#include "rstl/types.h"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/IConnection.hpp"
#include "Kyoto/TFunctor.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/list.hpp"

// A one-argument signal. The template name comes from the "TSignal1.h(56) : " allocation string;
// everything else is inferred from its instantiation for CDebugOption (CDebugOption.cpp and
// CGameDebug). Neither Echoes nor Prime has an equivalent.
//
// Each listener is a TFunctor1 kept in an rstl::list together with the connection object handed
// back to the caller. Deleting the connection removes the listener. Destroying the signal first
// detaches every connection, so connections that outlive the signal do nothing when deleted.

template < typename A1 >
class TSignal1 {
public:
  typedef TFunctor1< A1 > Functor;

  // Guessed name. One listener (0x1C bytes).
  struct SSlot {
    SSlot(IConnection* connection, const Functor& functor)
    : mConnection(connection), mFunctor(functor) {}

    IConnection* mConnection;
    Functor mFunctor;
  };

  typedef rstl::list< SSlot > SlotList;
  typedef typename SlotList::iterator iterator;
  typedef typename SlotList::const_iterator const_iterator;
  typedef TConnection< TSignal1, iterator > Connection;

  ~TSignal1() {
    for (iterator it = mSlots.begin(); it != mSlots.end(); ++it) {
      it->mConnection->Detach();
    }
  }

  // Guessed names.
  rstl::auto_ptr< IConnection > Connect(Functor functor) {
    Connection* connection =
        rs_new_line_in("TSignal1.h", 56) Connection(mSlots.end(), this, &TSignal1::Disconnect);
    connection->SetKey(mSlots.insert(mSlots.end(), SSlot(connection, functor)));
    return connection;
  }
  void Disconnect(iterator it) { mSlots.erase(it); }
  // Name confirmed by Metroid Prime Remastered symbols.
  void Emit(A1 arg) const {
    const_iterator end = mSlots.end();
    const_iterator it = mSlots.begin();
    while (it != end) {
      const_iterator slot = it;
      // Advance before calling the listener so it can disconnect itself.
      ++it;
      slot->mFunctor(arg);
    }
  }

private:
  SlotList mSlots;
};

#endif // _TSIGNAL1
