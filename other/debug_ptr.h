// Author: Stefano Brilli - stefanobrilli@gmail.com
// Licensing: see README file in the root dir of this project.
//
// This file contains the debug_ptr type and its helper classes.
//
// debug_ptr is a kind of smart pointer that may help to spot memory leaks in
// software by keeping track of the references to a dynamically allocated
// object (or array).
// When all references are lost, it checks if the object was deleted. If not,
// a Memory Leak event is constructed (depending on the choosen policy). 
//
// *******************************************
// **** USAGE **** look also at debug_ptr_test.cc:
// *******************************************
// 
// typedef_pointer(int*, int_p);
// typedef_array(int*, int_a);
//
// int_p p = new int(0);
// int_a a = new int[64];
//
// a[0] = 0;
// a[1] = 1;
// for (int i=2; i < 64; ++i)
//     a[i] = a[i-1] + a[i-2];
//
// *p = a[63];
// delete p;
// delete [] a;
//
// typedef_pointer and typedef_array macros are used to define an
// "alias" for standard pointer types. ie.
// When ENABLE_DEBUG_TYPES is not defined the first 2 lines corresponds to
//
// typedef int* int_p;
// typedef int* int_a;
//
// So that no overheads (either at compile or run time) are added.
// Instead, if ENABLE_DEBUG_TYPES is enabled, the first 2 lines corresponds to
//
// typedef debug_ptr<int*, false> int_p;
// typedef debug_ptr<int*, true> int_a;
//
// So that a check on the deletion of the pointer is performed during the
// runtime.
//
// *******************************************
// **** ADVANCED USAGE **** look at the source code
// *******************************************
//
// It may be necessary to retrieve and work with the standard pointer type inside
// a debug_ptr object, so 2 functions are provided:
//
// **** address_of ****
//
// int array[] = {0, 1, 2, 3, ... , 63}
// memcpy(debug_types::address_of(s), array, 64 * sizeof(int));
//
// if defined ENABLE_DEBUG_TYPES "s" is a debug_ptr type, and address_of returns
// a copy of the pointer inside the "s".
// Without the ENABLE_DEBUG_TYPES definition "s" is 
// just a standard pointer type, so address_of will return just "s";
//
// **** reset ****
// 
// int* tmp = address_of(s);
// delete [] tmp;
// debug_types::reset(s);
//
// If an object is deleted outside its debug pointer, it's necessary to warn
// its debug_ptr of the deletion. Reset will avoid a false Memory Leak detection
// given by the unawareness of the debug_ptr type about its object deletion
//

#ifndef DEBUG_PTR_H
#define DEBUG_PTR_H
#include <iostream>
#include <stdexcept>

#ifndef ENABLE_DEBUG_TYPES

namespace debug_types {
template <typename T> void reset(T& p) { p = NULL; }
template <typename T> void reset(T& p, T op) { p = op; }
template <typename T> T address_of(T p) { return p; }
}

#define typedef_pointer(Type, Alias, ...) typedef Type Alias
#define typedef_array(Type, Alias, ...) typedef Type Alias

#else // ENABLE_DEBUG_TYPES

// Defines the Policy class to call in case of Leak detection.
// This policy is overridable in typedef_* macros
#ifndef DEBUG_TYPES_POLICY
#define DEBUG_TYPES_POLICY PrintPolicy
#endif

#define DEBUG_TYPES_JOIN_ARGS(...) __VA_ARGS__

#define typedef_pointer(Type, Alias, ...) \
typedef debug_types::debug_ptr<Type, DEBUG_TYPES_JOIN_ARGS(false, ##__VA_ARGS__) > Alias

#define typedef_array(Type, Alias, ...) \
typedef debug_types::debug_ptr<Type, DEBUG_TYPES_JOIN_ARGS(true, ##__VA_ARGS__) > Alias

namespace debug_types {

// This policy will print a warning message in case of memory leak
struct PrintPolicy;
// This policy will throw a std::runtime_error in case of memory leak
struct ThrowPolicy;

// Declaration of the debug_ptr class. Note that this class is not actually defined.
// This is to avoid instantiation of debug_ptr for non pointer types.
template <typename T, bool Array=false, typename Policy=DEBUG_TYPES_POLICY> class debug_ptr;

// This is the forward declaration of the debug_ptr class for Pointer types.
template <typename T, bool Array, typename Policy> class debug_ptr<T*, Array, Policy>;

// This is the class that will delete the object and "mark" the object as deleted
// (by setting to NULL its pointer)
template <typename T, bool Array, typename Policy> class deleter;

// Marks as deleted the object referenced by a debug_ptr.
// WARNING: Use with care to avoid a false positive or a true negative 
// Memory Leak detections.
template <typename T_, bool Array_, typename Policy_> void reset(
    debug_ptr<T_*, Array_, Policy_>& p);
template <typename T_, bool Array_, typename Policy_> void reset(
    debug_ptr<T_*, Array_, Policy_>& p, T_* op);

// Returns the standard pointer inside a debug_ptr
template <typename T_, bool Array_, typename Policy_> T_* address_of(
    debug_ptr<T_*, Array_, Policy_>& p);
}

namespace debug_types {

struct ThrowPolicy {
    ThrowPolicy() {
        throw std::runtime_error("lost reference to undeleted object");
    }
};

struct PrintPolicy {
    PrintPolicy() {
        std::cerr << "WARNING: lost reference "
                     "to undeleted object" << std::endl;
    }
};

template <typename T_, bool Array_, typename Policy_> void reset(
    debug_ptr<T_*, Array_, Policy_>& p) {
    p.pd_->pointer_ = NULL;
};
template <typename T_, bool Array_, typename Policy_> void reset(
    debug_ptr<T_*, Array_, Policy_>& p, T_* op) {
    p.pd_->pointer_ = op;
}
template <typename T_, bool Array_, typename Policy_> T_* address_of(
    debug_ptr<T_*, Array_, Policy_>& p) {
    return p.pd_->pointer_;
};

namespace internal {

// An instance of this class is referenced by one or more debug_ptr(s).
// It contains the dynamically allocated object address and the number of
// debug_ptr(s) that are referencing this class instance.
// If the counter goes to 0 means that NO debug_ptr has any reference
// to this instance. Hence the object address is checked to see if 
// it was deleted and set to NULL by the deleter class. If the object
// was not deleted, the the Policy is called. Finally the instance delete
// itself.
template <typename T, typename Policy> class shared_ptr_data {
    // Only the deleter class and reset function can set to 0 the inner pointer
    friend class deleter<T, true, Policy>;
    friend class deleter<T, false, Policy>;
    template <typename T_, 
              bool Array_,
              typename Policy_> friend void debug_types::reset(
        debug_ptr<T_*, Array_, Policy_>& p);
    template <typename T_, 
              bool Array_,
              typename Policy_> friend void debug_types::reset(
        debug_ptr<T_*, Array_, Policy_>& p, T_* op);
    template <typename T_, 
              bool Array_,
              typename Policy_> friend T_* debug_types::address_of(
        debug_ptr<T_*, Array_, Policy_>& p);
public:

    static shared_ptr_data<T, Policy>* new_shared_ptr_data(T* ptr) {
        return new shared_ptr_data<T, Policy>(ptr, 0);
    }

    // Atomic increment
    // FIXME: if no threads are used, this could be just ++value_
    void IncrementRefCounter() {
        __sync_add_and_fetch(&value_, 1);
    }

    // Atomic decrement
    // FIXME: if no threads are used, this could be just --value_
    void DecrementRefCounter() {
        if (__sync_add_and_fetch(&value_, -1) == 0) {
            if (pointer_ != NULL)
                Policy();
            delete this;
        }
            
    }
    T* pointer() const { return pointer_; }

private:
    shared_ptr_data(T* ptr, int v) : pointer_(ptr), value_(v) {};
    shared_ptr_data(const shared_ptr_data<T, Policy>&);
    shared_ptr_data& operator=(const shared_ptr_data<T, Policy>&);

    T* pointer_;
    int value_;
};

} // namespace internal

// When deleter class is destroied it deletes the object referenced
// in its shared_ptr_data, then set to zero the address of that object
template <typename T, bool Array, typename Policy> class deleter {
public:
    static deleter<T, Array, Policy>* new_deleter(
            internal::shared_ptr_data<T, Policy>* spd) {
        if (Array) {
            deleter<T, Array, Policy>* tmp = new deleter<T, Array, Policy>[1];
            tmp[0].spd_ = spd;
            return tmp;
        } else {
            return new deleter<T, Array, Policy>(spd);
        }
    }

    ~deleter() {
        if (Array) {
            delete [] spd_->pointer();
        } else {
            delete spd_->pointer();
        }
        spd_->pointer_ = NULL;
    }

private:
    deleter() : spd_(NULL) {};
    deleter(internal::shared_ptr_data<T, Policy>* s) : spd_(s) {};
    deleter(const deleter<T, Array, Policy>&);
    deleter<T, Array, Policy>& operator=(const deleter&);

    internal::shared_ptr_data<T, Policy>* spd_;
};

// This class is the one that should replace the standard pointer type.
// Everytime that this class is copied or deleted, icreases or decreases 
// the reference counter inside its shared_ptr_data.
// when the delete or delete[] operator is called on this class, the cast
// to delete* operator is automatically called.
template <typename T, bool Array, typename Policy> class debug_ptr<T*, Array, Policy> {
public:

    template <typename T_, bool Array_, typename Policy_> friend void reset(
        debug_ptr<T_*, Array_, Policy_>& p);
    template <typename T_, bool Array_, typename Policy_> void reset(
        debug_ptr<T_*, Array_, Policy_>& p, T_* op);
    template <typename T_, bool Array_, typename Policy_> friend T_* address_of(
        debug_ptr<T_*, Array_, Policy_>& p);

    debug_ptr() {
        pd_ = internal::shared_ptr_data<T, Policy>::new_shared_ptr_data(NULL);
        pd_->IncrementRefCounter();
    }

    debug_ptr(T* ptr) {
        pd_ = internal::shared_ptr_data<T, Policy>::new_shared_ptr_data(ptr);
        pd_->IncrementRefCounter();
    }

    debug_ptr(const debug_ptr<T*, Array, Policy>& o) {
        o.pd_->IncrementRefCounter();
        pd_ = o.pd_;
    }

    ~debug_ptr() {
        pd_->DecrementRefCounter();
    }

    debug_ptr& operator=(const debug_ptr<T*, Array, Policy>& o) {
        o.pd_->IncrementRefCounter();
        pd_->DecrementRefCounter();
        pd_ = o.pd_;
        return *this;
    }

    debug_ptr& operator=(T* p) {
        debug_ptr<T*, Array, Policy>(p).swap(this);
        return *this;
    }
    
    bool operator<(const debug_ptr<T*, Array, Policy>& o) const { 
        return pd_->pointer() < o.pd_->pointer();
    }
    T& operator*() const { return *pd_->pointer(); }
    T* operator->() const { return pd_->pointer(); }
    T& operator[](int offset) const { return pd_->pointer()[offset]; }

    operator deleter<T, Array, Policy>*() const {
        return deleter<T, Array, Policy>::new_deleter(pd_);
    }
private:
    void swap(debug_ptr<T*, Array, Policy>* o) {
        internal::shared_ptr_data<T, Policy>* tmp = pd_;
        pd_ = o->pd_;
        o->pd_ = tmp;
    }

    internal::shared_ptr_data<T, Policy>* pd_;
};

}
#endif // ENABLE_DEBUG_TYPES

#endif