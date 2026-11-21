#ifndef _TSIGNAL4_HPP
#define _TSIGNAL4_HPP

#include "rstl/types.h"

#include "Kyoto/TFunctor.hpp"
#include "Kyoto/TSignal1.hpp"

#include "rstl/list.hpp"

// Guessed name
template < class R, class Arg1, class Arg2, class Arg3, class Arg4 >
class TFunctor4R {
public:
  typedef R (*Functor)(const void* object, const void* method, Arg1 arg1, Arg2 arg2, Arg3 arg3,
                       Arg4 arg4);

  TFunctor4R() : mFunctor(nullptr), mObject(nullptr) {}

  TFunctor4R(Functor functor, const void* object, const void* method, int size)
  : mFunctor(functor), mObject(object), mMethod(method, size) {}

  R operator()(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4) const {
    return mFunctor(mObject, mMethod.GetMethodPointer(), arg1, arg2, arg3, arg4);
  }

  operator bool() const { return !mMethod.IsNull(); }

private:
  Functor mFunctor;
  const void* mObject;
  CMethodPtrStore mMethod;
};

// Guessed name
template < class R, class A1, class A2, class A3, class A4 >
class TSignal4 {
public:
  typedef TFunctor4R< R, A1, A2, A3, A4 > Functor;

  struct SSlot {
    SSlot(IConnection* connection, const Functor& functor)
    : mConnection(connection), mFunctor(functor) {}

    IConnection* mConnection;
    Functor mFunctor;
  };

  typedef rstl::list< SSlot > SlotList;
  typedef typename SlotList::iterator iterator;
  typedef typename SlotList::const_iterator const_iterator;
  typedef TConnection< TSignal4, iterator > Connection;

  // Every connection forgets the signal before the slots go away.
  ~TSignal4() {
    for (iterator it = mSlots.begin(); it != mSlots.end(); ++it) {
      it->mConnection->Detach();
    }
  }

  bool HasListeners() const { return mSlots.size() != 0; }
  rstl::auto_ptr< IConnection > Connect(Functor functor) {
    Connection* connection =
        rs_new_line_in("TSignal4R.h", 62) Connection(mSlots.end(), this, &TSignal4::Disconnect);
    connection->SetKey(mSlots.insert(mSlots.end(), SSlot(connection, functor)));
    return connection;
  }
  void Disconnect(iterator it) { mSlots.erase(it); }
  R Emit(A1 a1, A2 a2, A3 a3, A4 a4) const;

private:
  SlotList mSlots;
};

// Each slot's iterator is advanced before its functor runs, so a slot may disconnect itself.
template < class R, class A1, class A2, class A3, class A4 >
R TSignal4< R, A1, A2, A3, A4 >::Emit(A1 a1, A2 a2, A3 a3, A4 a4) const {
  const_iterator end = mSlots.end();
  const_iterator it = mSlots.begin();
  while (it != end) {
    const_iterator slot = it;
    ++it;
    if (it == end) {
      return slot->mFunctor(a1, a2, a3, a4);
    }
    slot->mFunctor(a1, a2, a3, a4);
  }
  return R();
}

// A void signal invokes every listener without collecting a return value.
template < class A1, class A2, class A3, class A4 >
class TSignal4< void, A1, A2, A3, A4 > {
public:
  typedef TFunctor4R< void, A1, A2, A3, A4 > Functor;

  struct SSlot {
    SSlot(IConnection* connection, const Functor& functor)
    : mConnection(connection), mFunctor(functor) {}

    IConnection* mConnection;
    Functor mFunctor;
  };

  typedef rstl::list< SSlot > SlotList;
  typedef typename SlotList::iterator iterator;
  typedef typename SlotList::const_iterator const_iterator;
  typedef TConnection< TSignal4, iterator > Connection;

  // Every connection forgets the signal before the slots go away.
  ~TSignal4() {
    for (iterator it = mSlots.begin(); it != mSlots.end(); ++it) {
      it->mConnection->Detach();
    }
  }

  bool HasListeners() const { return mSlots.size() != 0; }
  rstl::auto_ptr< IConnection > Connect(Functor functor) {
    Connection* connection =
        rs_new_line_in("TSignal4.h", 58) Connection(mSlots.end(), this, &TSignal4::Disconnect);
    connection->SetKey(mSlots.insert(mSlots.end(), SSlot(connection, functor)));
    return connection;
  }
  void Disconnect(iterator it) { mSlots.erase(it); }
  void Emit(A1 a1, A2 a2, A3 a3, A4 a4) const;

private:
  SlotList mSlots;
};

// Each slot's iterator is advanced before its functor runs, so a slot may disconnect itself.
template < class A1, class A2, class A3, class A4 >
inline void TSignal4< void, A1, A2, A3, A4 >::Emit(A1 a1, A2 a2, A3 a3, A4 a4) const {
  const_iterator end = mSlots.end();
  const_iterator it = mSlots.begin();
  while (it != end) {
    const_iterator slot = it;
    ++it;
    slot->mFunctor(a1, a2, a3, a4);
  }
}

// Guessed name
template < class T, class R, class A1, class A2, class A3, class A4 >
class TNonStaticCallback4R {
public:
  typedef R (T::*MethodPtr)(A1, A2, A3, A4);

  static R Function(const void* object, const void* method, A1 a1, A2 a2, A3 a3, A4 a4) {
    MethodPtr callback;
    memcpy(&callback, method, sizeof(callback));
    return (static_cast< T* >(const_cast< void* >(object))->*callback)(a1, a2, a3, a4);
  }
};

// Guessed name
template < class T, class R, class A1, class A2, class A3, class A4 >
class TFunctor4RFromMethod {
public:
  typedef R (T::*MethodPtr)(A1, A2, A3, A4);

  static TFunctor4R< R, A1, A2, A3, A4 > Make(T& object, MethodPtr method) {
    char methodData[sizeof(method)];
    memcpy(methodData, &method, sizeof(method));
    return TFunctor4R< R, A1, A2, A3, A4 >(TNonStaticCallback4R< T, R, A1, A2, A3, A4 >::Function,
                                           &object, methodData, sizeof(method));
  }
};

#endif // _TSIGNAL4_HPP
