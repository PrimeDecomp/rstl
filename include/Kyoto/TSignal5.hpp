#ifndef _TSIGNAL5_HPP
#define _TSIGNAL5_HPP

#include "rstl/types.h"

#include "Kyoto/IConnection.hpp"
#include "Kyoto/TFunctor.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/list.hpp"

// Guessed name
template < class Arg1, class Arg2, class Arg3, class Arg4, class Arg5 >
class TFunctor5 {
public:
  typedef void (*Functor)(const void* object, const void* method, Arg1 arg1, Arg2 arg2, Arg3 arg3,
                          Arg4 arg4, Arg5 arg5);

  TFunctor5() : mFunctor(nullptr), mObject(nullptr) {}

  TFunctor5(Functor functor, const void* object, const void* method, int size)
  : mFunctor(functor), mObject(object), mMethod(method, size) {}

  void operator()(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5) const {
    mFunctor(mObject, mMethod.GetMethodPointer(), arg1, arg2, arg3, arg4, arg5);
  }

private:
  Functor mFunctor;
  const void* mObject;
  CMethodPtrStore mMethod;
};

// The five-argument void signal, shaped like TSignal4 (the asserts name TSignal1.h .. TSignal5.h).
// CPlayer's user animation event signal (0x3258) is the instance modelled so far.
template < class A1, class A2, class A3, class A4, class A5 >
class TSignal5 {
public:
  typedef TFunctor5< A1, A2, A3, A4, A5 > Functor;

  struct SSlot {
    SSlot(IConnection* connection, const Functor& functor)
    : mConnection(connection), mFunctor(functor) {}

    IConnection* mConnection;
    Functor mFunctor;
  };

  typedef rstl::list< SSlot > SlotList;
  typedef typename SlotList::iterator iterator;
  typedef typename SlotList::const_iterator const_iterator;
  typedef TConnection< TSignal5, iterator > Connection;

  // Every connection forgets the signal before the slots go away.
  ~TSignal5() {
    for (iterator it = mSlots.begin(); it != mSlots.end(); ++it) {
      it->mConnection->Detach();
    }
  }

  rstl::auto_ptr< IConnection > Connect(Functor functor) {
    Connection* connection =
        rs_new_line_in("TSignal5.h", 59) Connection(mSlots.end(), this, &TSignal5::Disconnect);
    connection->SetKey(mSlots.insert(mSlots.end(), SSlot(connection, functor)));
    return connection;
  }
  void Disconnect(iterator it) { mSlots.erase(it); }
  void Emit(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) const;

private:
  SlotList mSlots;
};

// Each slot's iterator is advanced before its functor runs, so a slot may disconnect itself.
template < class A1, class A2, class A3, class A4, class A5 >
void TSignal5< A1, A2, A3, A4, A5 >::Emit(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) const {
  const_iterator end = mSlots.end();
  const_iterator it = mSlots.begin();
  while (it != end) {
    const_iterator slot = it;
    ++it;
    slot->mFunctor(a1, a2, a3, a4, a5);
  }
}

// Guessed names. The callback stores a copied member-function pointer like the
// other signal arities; the fifth argument is a float for the player event signal.
template < class T, class A1, class A2, class A3, class A4, class A5 >
class TNonStaticCallback5 {
public:
  typedef void (T::*MethodPtr)(A1, A2, A3, A4, A5);

  static void Function(const void* object, const void* method, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    MethodPtr callback;
    memcpy(&callback, method, sizeof(callback));
    (static_cast< T* >(const_cast< void* >(object))->*callback)(a1, a2, a3, a4, a5);
  }
};

template < class T, class A1, class A2, class A3, class A4, class A5 >
class TFunctor5FromMethod {
public:
  typedef void (T::*MethodPtr)(A1, A2, A3, A4, A5);

  static TFunctor5< A1, A2, A3, A4, A5 > Make(T& object, MethodPtr method) {
    char methodData[sizeof(method)];
    memcpy(methodData, &method, sizeof(method));
    return TFunctor5< A1, A2, A3, A4, A5 >(TNonStaticCallback5< T, A1, A2, A3, A4, A5 >::Function,
                                           &object, methodData, sizeof(method));
  }
};

#endif // _TSIGNAL5_HPP
