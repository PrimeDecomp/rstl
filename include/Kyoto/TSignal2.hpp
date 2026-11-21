#ifndef _TSIGNAL2_HPP
#define _TSIGNAL2_HPP

#include "rstl/types.h"

#include "Kyoto/Alloc/CMemory.hpp"

#include "Kyoto/TFunctor.hpp"
#include "Kyoto/IConnection.hpp"

#include "rstl/list.hpp"
#include "rstl/auto_ptr.hpp"

template < class A1, class A2 >
class TSignal2 {
public:
  struct SSlot {
    SSlot(IConnection* connection, const TFunctor2< A1, A2 >& functor)
    : mConnection(connection), mFunctor(functor) {}

    IConnection* mConnection;
    TFunctor2< A1, A2 > mFunctor;
  };

  typedef TFunctor2< A1, A2 > Functor;
  typedef rstl::list< SSlot > SlotList;
  typedef typename SlotList::iterator iterator;
  typedef TConnection< TSignal2, iterator > Connection;

  ~TSignal2() {
    for (iterator it = mSlots.begin(); it != mSlots.end(); ++it) {
      it->mConnection->Detach();
    }
  }

  // Guessed name
  rstl::auto_ptr< IConnection > Connect(Functor functor) {
    Connection* connection =
        rs_new_line_in("TSignal2.h", 55) Connection(mSlots.end(), this, &TSignal2::Disconnect);
    connection->SetKey(mSlots.insert(mSlots.end(), SSlot(connection, functor)));
    return connection;
  }
  void Disconnect(iterator it) { mSlots.erase(it); } // Guessed name

  // Each slot's iterator is advanced before its functor runs, so a slot may disconnect itself.
  void Emit(A1 a1, A2 a2) const {
    typename SlotList::const_iterator end = mSlots.end();
    typename SlotList::const_iterator it = mSlots.begin();
    while (it != end) {
      typename SlotList::const_iterator slot = it;
      ++it;
      slot->mFunctor(a1, a2);
    }
  }

private:
  SlotList mSlots;
};

#endif // _TSIGNAL2_HPP
