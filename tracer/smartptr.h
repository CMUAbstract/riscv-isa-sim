#ifndef SMART_PTR_H
#define SMART_PTR_H

#include <algorithm>

// This is custom because normal shared pointers are really slow
template<typename T>
class shared_ptr_t {
public:
	// Explicit constructor
	explicit shared_ptr_t(T* data) : data(data), count(new uint32_t(1)) {}
	~shared_ptr_t() {
		--(*count);
		if (*count == 0) {
			delete data;
			delete count;
		}
	}
	shared_ptr_t(shared_ptr_t const& copy) : data(copy.data), count(copy.count) {
		++(*count);
	}
	// Use the copy and swap idiom
	// It works perfectly for this situation.
	shared_ptr_t& operator=(shared_ptr_t rhs) {
		rhs.swap(*this);
		return *this;
	}
	shared_ptr_t& operator=(T* newData) {
		shared_ptr_t tmp(newData);
		tmp.swap(*this);
		return *this;
	}

	// Always good to have a swap function
	// Make sure it is noexcept
	void swap(shared_ptr_t& other) noexcept {
		std::swap(data,  other.data);
		std::swap(count, other.count);
	}

	// Const correct access owned object
	T* operator->() const {return data;}
	T& operator*()  const {return *data;}

	// Access to smart pointer state
	T* get() const {return data;}
	explicit operator bool() const {return data;}
private:
	T* data;
	uint32_t* count;
};

#endif