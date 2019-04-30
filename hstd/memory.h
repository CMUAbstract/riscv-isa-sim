#ifndef MEMORY_H
#define MEMORY_H

#include <cstdint>
#include <algorithm>

namespace hstd {

	// This is custom because normal shared pointers are really slow

	template<typename T>
	class shared_ptr {
	public:
		// Explicit constructor
		explicit shared_ptr() : data(nullptr), count(new uint32_t(1)) {}
		explicit shared_ptr(T* data) : data(data), count(new uint32_t(1)) {}
		~shared_ptr() {
			--(*count);
			if (*count == 0) {
				if(data != nullptr) delete data;
				delete count;
			}
		}
		shared_ptr(shared_ptr const& copy) : data(copy.data), count(copy.count) {
			++(*count);
		}
		// Use the copy and swap idiom
		// It works perfectly for this situation.
		shared_ptr& operator=(shared_ptr rhs) {
			rhs.swap(*this);
			return *this;
		}
		shared_ptr& operator=(T* newData) {
			shared_ptr tmp(newData);
			tmp.swap(*this);
			return *this;
		}

		// Always good to have a swap function
		// Make sure it is noexcept
		void swap(shared_ptr& other) noexcept {
			std::swap(data,  other.data);
			std::swap(count, other.count);
		}

		// Const correct access owned object
		T* operator->() const {return data;}
		T& operator*()  const {return *data;}
		bool operator==(const shared_ptr<T> &other) { return data == other.get(); }
		bool operator<(const shared_ptr<T> &other) { return data < other.get(); }

		// Access to smart pointer state
		T* get() const {return data;}
		explicit operator bool() const {return data;}
	protected:
		T* data;
		uint32_t* count;
	};

	template<typename T> struct is_shared_ptr : std::false_type {};
	template<typename T> struct is_shared_ptr<shared_ptr<T>> : std::true_type {};

}

#endif