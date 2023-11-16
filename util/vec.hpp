#ifndef VEC_HPP
#define VEC_HPP

// Vector typedefs

#include <vector>
//#include "./allocate.hpp"

template <typename T>
using managed_vec = std::vector<T>;
//using managed_vec = std::vector<T, allocator<T>>; // There used to be a custom allocator but I got rid of it because I did not observe any benefits after switching to it

/*

I had experimented with my own vector class with the goal of finding the 
perfect values of VEC_DEFAULT_ALLOC and VEC_DEFAULT_REALLOC_FACTOR. However,
after some experimentation I found that my own vector implementation
basically has identical performance to the std::vector. I still use my own
version though for the sake of simplicity.

*/

#define USE_STD_VECTOR

#ifdef USE_STD_VECTOR
template <typename T>
using vec = std::vector<T>; // Regular vector with default allocator
#else

#include <initializer_list>
#include <iterator>
#include <cstddef>

#define VEC_DEFAULT_ALLOC 1
#define VEC_DEFAULT_REALLOC_FACTOR 2.0

struct VecSubscriptOutOfRange { };

template <typename T>
struct vec_iterator {

	using iterator_category = std::forward_iterator_tag;
	using difference_type = std::ptrdiff_t;
	using value_type = T;
	using pointer = T*;
	using reference = T&;

	vec_iterator(pointer ptr);

	reference operator * () const;
	pointer operator -> () const;

	vec_iterator operator ++ ();

	pointer ptr;

};

template <typename T>
struct vec {

	vec();
	vec(int);
	vec(std::initializer_list<T>);
	vec(const vec&);

	void operator = (const vec&);

	T& operator [] (int) const;

	int size() const;
	void push_back(T);
	void resize(int);
	void clear();

	bool operator == (const vec&) const;
	bool operator != (const vec&) const;
	bool operator > (const vec&) const;
	bool operator < (const vec&) const;
	bool operator >= (const vec&) const;
	bool operator <= (const vec&) const;

	~vec();

	using Iterator = vec_iterator<T>;

	Iterator begin() const;
	Iterator end() const;

protected:

	T default_mem[VEC_DEFAULT_ALLOC];
	T* mem = default_mem;
	int s = 0;
	int capacity = VEC_DEFAULT_ALLOC;

	void reallocate(int, bool = false);
	void copy(const vec&);
	void cleanup();

};

template <typename T>
vec_iterator<T>::vec_iterator(pointer ptr) : ptr(ptr) { }

template <typename T>
T& vec_iterator<T>::operator * () const { return *ptr; }

template <typename T>
T* vec_iterator<T>::operator -> () const { return ptr; }

template <typename T>
vec_iterator<T> vec_iterator<T>::operator ++ () { ptr++; return *this; }

template <typename T>
bool operator == (const vec_iterator<T>& a, const vec_iterator<T>& b) { return a.ptr == b.ptr; }

template <typename T>
bool operator != (const vec_iterator<T>& a, const vec_iterator<T>& b) { return a.ptr != b.ptr; }

template <typename T>
vec_iterator<T> vec<T>::begin() const { return Iterator{ mem }; }

template <typename T>
vec_iterator<T> vec<T>::end() const { return Iterator{ mem + s }; }

template <typename T>
vec<T>::vec() { }

template <typename T>
vec<T>::vec(int cap) { s = cap; if (cap > VEC_DEFAULT_ALLOC) reallocate(cap); }

template <typename T>
vec<T>::vec(std::initializer_list<T> init) {

	s = (int)init.size();
	if (s > VEC_DEFAULT_ALLOC) reallocate(s);

	for (int i = 0; i < s; i++) mem[i] = init.begin()[i];

}

template <typename T>
vec<T>::vec(const vec<T>& v) { copy(v); }

template <typename T>
void vec<T>::operator = (const vec<T>& v) { copy(v); }

template <typename T>
T& vec<T>::operator [] (int i) const { if (i < 0 || i >= s) throw VecSubscriptOutOfRange{ }; return mem[i]; }

template <typename T>
int vec<T>::size() const { return s; }

template <typename T>
void vec<T>::push_back(T x) { if (s == capacity) reallocate(capacity, true); mem[s] = x; s++; }

template <typename T>
void vec<T>::resize(int s_new) { if (s_new > capacity) reallocate(s_new, true); s = s_new; }

template <typename T>
void vec<T>::clear() { s = 0; }

template <typename T>
bool vec<T>::operator == (const vec& v) const {

	if (s != v.s) return false;
	for (int i = 0; i < s; i++) if (mem[i] != v.mem[i]) return false;
	return true;

}

template <typename T>
bool vec<T>::operator != (const vec& v) const { return !(*this == v); }

template <typename T>
bool vec<T>::operator > (const vec& v) const {

#define VEC_GT_COMMON_CODE() \
\
int s_min = s < v.s ? s : v.s; \
\
for (int i = 0; i < s; i++) { \
	\
	if (mem[i] > v.mem[i]) return true; \
	if (mem[i] < v.mem[i]) return false; \
	\
} \
\
return

	VEC_GT_COMMON_CODE() false;

}

template <typename T>
bool vec<T>::operator < (const vec& v) const { return v > * this; }

template <typename T>
bool vec<T>::operator >= (const vec& v) const { VEC_GT_COMMON_CODE() true; }

template <typename T>
bool vec<T>::operator <= (const vec& v) const { return v >= *this; }

template <typename T>
vec<T>::~vec() { cleanup(); }

template <typename T>
void vec<T>::reallocate(int new_cap, bool memcopy) {

	new_cap = 1 + (int)((float)new_cap * VEC_DEFAULT_REALLOC_FACTOR);

	T* new_mem = new T[new_cap];

	if (memcopy) {

		int min_size = new_cap < capacity ? new_cap : capacity;
		for (int i = 0; i < min_size; i++) new_mem[i] = mem[i];

	}

	if (mem != default_mem) delete[] mem;
	mem = new_mem;
	capacity = new_cap;

}

template <typename T>
void vec<T>::copy(const vec<T>& v) { s = v.s; reallocate(v.capacity); for (int i = 0; i < s; i++) mem[i] = v.mem[i]; }

template <typename T>
void vec<T>::cleanup() { if (mem != default_mem) delete[] mem; }

#endif

#endif