// Author: Stefano Brilli - stefanobrilli@gmail.com
// Licensing: see README file in the root dir of this project.
//
// debug_ptr is a class designed to be used in place of standard pointers
// for storing the address of dinamically allocated Objects and Object arrays.
// It helps to spot memory leaks in your software by
// keeping track of the references to a dynamically allocated object (or array)
// and, when all references are lost, checks if the object was deleted. If not,
// an event is raised (depending on the choosen policy). 

#ifndef DEBUG_PTR_H
#define DEBUG_PTR_H
#include <iostream>
#include <stdexcept>

#ifndef ENABLE_DEBUG_TYPES
#define typedef_pointer(Type, Alias, ...) typedef Type Alias
#define typedef_array(Type, Alias, ...) typedef Type Alias

#else // ENABLE_DEBUG_TYPES

// Defines the Policy class to call in case of Leak detection
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
// This forward declares the debug_ptr class for Pointer types.
template <typename T, bool Array, typename Policy> class debug_ptr<T*, Array, Policy>;

// This is the class that will delete the object and "mark" the object as deleted
// (by setting to 0 its )
template <typename T, bool Array, typename Policy> class deleter;
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

namespace internal {

// An instance of this class is referenced by one or more debug_ptr(s).
// It contains the dynamically allocated object addess and a counter of
// debug_ptr(s) that are referencing the class instance.
// If the counter goes to 0 means that no debug_ptr has any reference
// to this instance. Hence the object address is checked to see if 
// it was set to NULL (and deleted) by the deleter class. If the object
// was not deleted, the the Policy is called. Finally the instance delete
// itself.

template <typename T, typename Policy> class shared_ptr_data {
	// Only the deleter class can set to 0 the inner pointer
	friend class deleter<T, true, Policy>;
	friend class deleter<T, false, Policy>;
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
	void set_pointer(T* ptr) { pointer_ = ptr; }

private:
	shared_ptr_data(T* ptr, int c) : pointer_(ptr), value_(c) {}
	shared_ptr_data(const shared_ptr_data<T, Policy>&);

	shared_ptr_data& operator=(const shared_ptr_data<T, Policy>&);

	T* pointer_;
	int value_;
};

} // namespace internal

// When deleter class is destroied it deletes the object referenced
// in its shared_ptr_data. then Set to zero the address of that object
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

// This class is the one that should replace the real pointer type.
// Everytime that this class is copied or deleted, icreases or decreases 
// the reference counter inside its shared_ptr_data.
// when the delete or delete [] operator is called on this class, the cast
// to delete* operator is automatically called.
template <typename T, bool Array, typename Policy> class debug_ptr<T*, Array, Policy> {
public:

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

	~debug_ptr() {
		pd_->DecrementRefCounter();
	}

	T& operator*() { return *pd_->pointer(); }
	const T& operator*() const { return *pd_->pointer(); }
	T* operator->() { return pd_->pointer(); }
	const T* operator->() const { return pd_->pointer(); }
	T& operator[](int offset) { return pd_->pointer()[offset]; }
	const T& operator[](int offset) const { return pd_->pointer()[offset]; }

	operator deleter<T, Array, Policy>*() {
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