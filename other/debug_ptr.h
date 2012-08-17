#ifndef DEBUG_PTR_H
#define DEBUG_PTR_H
#include <iostream>

namespace debug_types {
template <typename T> class debug_ptr;
template <typename T> class debug_array;
}

#ifdef ENABLE_DEBUG_TYPES
#define typedef_pointer(Type, Alias) typedef debug_types::debug_ptr< Type > Alias
#define typedef_array(Type, Alias) typedef debug_types::debug_array< Type > Alias
#else
#define typedef_pointer(Type, Alias) typedef Type* Alias
#define typedef_array(Type, Alias) typedef Type* Alias
#endif


namespace debug_types {
namespace internal {
template <typename T> class shared_ptr_data {
public:
	static shared_ptr_data<T>* new_shared_ptr_data(T* ptr) {
		return new shared_ptr_data<T>(ptr, 0);
	}
	void IncrementRefCounter() {
		__sync_add_and_fetch(&value_, 1);
	}
	void DecrementRefCounter() {
		if (__sync_add_and_fetch(&value_, -1) == 0) {
			if (pointer_ != NULL)
				throw;
			delete this;
		}
			
	}
	T* pointer() const { return pointer_; }
	void set_pointer(T* ptr) { pointer_ = ptr; }

private:
	shared_ptr_data(T* ptr, int c) : pointer_(ptr), value_(c) {}
	shared_ptr_data(const shared_ptr_data<T>&);

	shared_ptr_data& operator=(const shared_ptr_data<T>&);

	T* pointer_;
	int value_;
};
} // End of namespace private

template <typename T> class ptr_deleter {
public:
	static ptr_deleter<T>* new_ptr_deleter(internal::shared_ptr_data<T>* spd) {
		return new ptr_deleter<T>(spd);
	}

	~ptr_deleter() {
		delete spd_->pointer();
		spd_->set_pointer(NULL);
	}

private:
	ptr_deleter() : spd_(NULL) {};
	ptr_deleter(internal::shared_ptr_data<T>* s) : spd_(s) {};
	ptr_deleter(const ptr_deleter<T>&);
	ptr_deleter<T>& operator=(const ptr_deleter&);

	internal::shared_ptr_data<T>* spd_;
};

template <typename T> class debug_ptr {
public:

	debug_ptr(T* ptr) {
		pd_ = internal::shared_ptr_data<T>::new_shared_ptr_data(ptr);
		pd_->IncrementRefCounter();
	}

	debug_ptr(const debug_ptr<T>& o) {
		o.pd_->IncrementRefCounter();
		pd_ = o.pd_;
	}

	debug_ptr& operator=(const debug_ptr<T>& o) {
		o.pd_->IncrementRefCounter();
		pd_->DecrementRefCounter();
		pd_ = o.pd_;
		return *this;
	}

	debug_ptr& operator=(T* p) {
		debug_ptr<T>(p).swap(this);
		return *this;
	}

	~debug_ptr() {
		pd_->DecrementRefCounter();
	}

	T& operator*() { return *pd_->pointer(); }
	const T& operator*() const { return *pd_->pointer(); }
	T* operator->() { return pd_->pointer(); }
	const T* operator->() const { return pd_->pointer(); }

	operator ptr_deleter<T>*() {
		return ptr_deleter<T>::new_ptr_deleter(pd_);
	}

private:

	void swap(debug_ptr<T>* o) {
		internal::shared_ptr_data<T>* tmp = pd_;
		pd_ = o->pd_;
		o->pd_ = tmp;
	}

	internal::shared_ptr_data<T>* pd_;

};


template <typename T> class array_deleter {
public:
	static array_deleter<T>* new_array_deleter(internal::shared_ptr_data<T>* spd) {
		array_deleter<T>* tmp = new array_deleter<T>[1];
		tmp[0].spd_ = spd;
		return tmp;
	}

	~array_deleter() {
		delete [] spd_->pointer();
		spd_->set_pointer(NULL);
	}

private:
	array_deleter() : spd_(NULL) {};
	array_deleter(internal::shared_ptr_data<T>* s) : spd_(s) {};
	array_deleter(const array_deleter<T>&);
	array_deleter<T>& operator=(const array_deleter&);

	internal::shared_ptr_data<T>* spd_;
};

template <typename T> class debug_array {
public:

	debug_array(T* ptr) {
		pd_ = internal::shared_ptr_data<T>::new_shared_ptr_data(ptr);
		pd_->IncrementRefCounter();
	}

	debug_array(const debug_array<T>& o) {
		o.pd_->IncrementRefCounter();
		pd_ = o.pd_;
	}

	debug_array& operator=(const debug_array<T>& o) {
		o.pd_->IncrementRefCounter();
		pd_->DecrementRefCounter();
		pd_ = o.pd_;
		return *this;
	}

	debug_array& operator=(T* p) {
		debug_array<T>(p).swap(this);
		return *this;
	}

	~debug_array() {
		pd_->DecrementRefCounter();
	}

	T& operator*() { return *pd_->pointer(); }
	const T& operator*() const { return *pd_->pointer(); }
	T* operator->() { return pd_->pointer(); }
	const T* operator->() const { return pd_->pointer(); }
	T& operator[](int offset) { return pd_->pointer()[offset]; }
	const T& operator[](int offset) const { return pd_->pointer()[offset]; }

	operator array_deleter<T>*() {
		return array_deleter<T>::new_array_deleter(pd_);
	}

private:

	void swap(debug_array<T>* o) {
		internal::shared_ptr_data<T>* tmp = pd_;
		pd_ = o->pd_;
		o->pd_ = tmp;
	}

	internal::shared_ptr_data<T>* pd_;

};

}
#endif