#ifndef _TSIGNAL1
#define _TSIGNAL1

#include "rstl/types.h"
#include "rstl/static_assert.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/IConnection.hpp"
#include "Kyoto/TFunctor.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/list.hpp"

template < typename A1 >
class TSignal1 {
public:
  typedef TFunctor1< A1 > Functor;

  // Guessed name
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

NESTED_CHECK_SIZEOF(TSignal1< int >, SSlot, 0x1C)

#endif // _TSIGNAL1
