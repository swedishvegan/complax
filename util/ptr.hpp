#ifndef PTR_HPP
#define PTR_HPP

// Custom smart pointer implementation (this was before I knew about std smart pointers)

#define USE_SMART_POINTER

#ifdef USE_SMART_POINTER

template <int _>
struct ptr_container_base {

	static int num_allocs;

	const void* p = nullptr;
	int ref_count = 0;

	ptr_container_base(const void*);

};

template <int _>
int ptr_container_base<_>::num_allocs = 0;

using ptr_container = ptr_container_base<0>;

template <typename ptr_type>
struct ptr {

	template <typename cast_type>
	inline ptr<cast_type> cast();

	inline ptr();
	inline ptr(const ptr&);
	inline ptr(const ptr_type*);

	inline void operator = (const ptr&);
	inline void operator = (const ptr_type*);

	inline ptr_type* operator -> () const;
	inline ptr_type& operator * () const;

	inline ptr_type* operator () () const;

	bool operator == (const ptr&) const;
	bool operator == (ptr_type*) const;

	bool operator != (const ptr&) const;
	bool operator != (ptr_type*) const;

	bool operator >= (const ptr&) const;
	bool operator >= (ptr_type*) const;

	bool operator <= (const ptr&) const;
	bool operator <= (ptr_type*) const;

	bool operator > (const ptr&) const;
	bool operator > (ptr_type*) const;

	bool operator < (const ptr&) const;
	bool operator < (ptr_type*) const;

	~ptr();

protected:

	ptr_container* ptr_cnt = nullptr;

	ptr(ptr_container*, void*);

	void copy(const ptr&);
	void init(const ptr_type*);
	void cleanup();

	template <typename friend_type>
	friend struct ptr;

};

template <int _>
ptr_container_base<_>::ptr_container_base(const void* p) : p(p) { }

template <typename ptr_type>
template <typename cast_type>
inline ptr<cast_type> ptr<ptr_type>::cast() { return ptr_cnt ? ptr<cast_type>(ptr_cnt, nullptr) : ptr<cast_type>(); }

template <typename ptr_type>
inline ptr<ptr_type>::ptr() { init(nullptr); }

template <typename ptr_type>
inline ptr<ptr_type>::ptr(const ptr& ptr) { copy(ptr); }

template <typename ptr_type>
inline ptr<ptr_type>::ptr(const ptr_type* ptr) { init(ptr); }

template <typename ptr_type>
inline void ptr<ptr_type>::operator = (const ptr& ptr) { cleanup(); copy(ptr); }

template <typename ptr_type>
inline void ptr<ptr_type>::operator = (const ptr_type* ptr) { cleanup(); init(ptr); }

template <typename ptr_type>
inline ptr_type* ptr<ptr_type>::operator -> () const { return (ptr_type*)ptr_cnt->p; }

template <typename ptr_type>
inline ptr_type& ptr<ptr_type>::operator * () const { return *(ptr_type*)(ptr_cnt->p); }

template <typename ptr_type>
inline ptr_type* ptr<ptr_type>::operator () () const { if (ptr_cnt) if (ptr_cnt->p) return (ptr_type*)ptr_cnt->p; return nullptr; }

template <typename ptr_type>
bool ptr<ptr_type>::operator == (const ptr& rh) const { return ptr_cnt ? (rh.ptr_cnt ? ptr_cnt->p == rh.ptr_cnt->p : false) : !rh.ptr_cnt; }

template <typename ptr_type>
bool ptr<ptr_type>::operator == (ptr_type* rh) const { return ptr_cnt ? ptr_cnt->p == rh : !rh; }

template <typename ptr_type>
bool ptr<ptr_type>::operator != (const ptr& rh) const { return ptr_cnt ? (rh.ptr_cnt ? ptr_cnt->p != rh.ptr_cnt->p : true) : (bool)rh.ptr_cnt; }

template <typename ptr_type>
bool ptr<ptr_type>::operator != (ptr_type* rh) const { return ptr_cnt ? ptr_cnt->p != rh : (bool)rh; }

template <typename ptr_type>
bool ptr<ptr_type>::operator >= (const ptr& rh) const {

	if (!ptr_cnt && !rh.ptr_cnt) return true;
	if (!ptr_cnt && rh.ptr_cnt) return false;
	if (ptr_cnt && !rh.ptr_cnt) return true;
	return ptr_cnt->p >= rh.ptr_cnt->p;

}

template <typename ptr_type>
bool ptr<ptr_type>::operator >= (ptr_type* rh) const { if (!ptr_cnt && rh) return false; return ptr_cnt->p >= (const void*)rh; }

template <typename ptr_type>
bool ptr<ptr_type>::operator <= (const ptr& rh) const { return rh >= *this; }

template <typename ptr_type>
bool ptr<ptr_type>::operator <= (ptr_type* rh) const { if (!ptr_cnt && rh) return true; return ptr_cnt->p <= (const void*)rh; }


template <typename ptr_type>
bool ptr<ptr_type>::operator > (const ptr& rh) const {

	if (!ptr_cnt && !rh.ptr_cnt) return false;
	if (!ptr_cnt && rh.ptr_cnt) return false;
	if (ptr_cnt && !rh.ptr_cnt) return true;
	return ptr_cnt->p > rh.ptr_cnt->p;

}

template <typename ptr_type>
bool ptr<ptr_type>::operator > (ptr_type* rh) const { if (!ptr_cnt && rh) return false; return ptr_cnt->p > (const void*)rh; }

template <typename ptr_type>
bool ptr<ptr_type>::operator < (const ptr& rh) const { return rh > * this; }

template <typename ptr_type>
bool ptr<ptr_type>::operator < (ptr_type* rh) const { if (!ptr_cnt && rh) return true; return ptr_cnt->p < (const void*)rh; }

template <typename ptr_type>
ptr<ptr_type>::~ptr() { cleanup(); }

template <typename ptr_type>
ptr<ptr_type>::ptr(ptr_container* ptr_cnt, void*) : ptr_cnt(ptr_cnt) { ptr_cnt->ref_count++; }

template <typename ptr_type>
void ptr<ptr_type>::copy(const ptr& ptr) {

	if (ptr.ptr_cnt == nullptr) { ptr_cnt = nullptr; return; }

	ptr_cnt = ptr.ptr_cnt;
	ptr_cnt->ref_count++;
}

template <typename ptr_type>
void ptr<ptr_type>::init(const ptr_type* ptr) {

	if (ptr) { ptr_cnt = new ptr_container(ptr); ptr_container::num_allocs++; }
	else ptr_cnt = nullptr;

}

template <typename ptr_type>
void ptr<ptr_type>::cleanup() {

	if (!ptr_cnt) return;

	if (ptr_cnt->ref_count == 0) { if (ptr_cnt->p) delete (const ptr_type*)ptr_cnt->p; delete ptr_cnt; ptr_container::num_allocs--; }
	else ptr_cnt->ref_count--;

}

#else

template <typename ptr_type>
struct ptr {

	template <typename cast_type>
	ptr<cast_type> cast();

	ptr();
	ptr(const ptr&);
	ptr(const ptr_type*);

	inline void operator = (const ptr&);
	inline void operator = (const ptr_type*);

	inline ptr_type* operator -> () const;
	inline ptr_type& operator * () const;

	inline ptr_type* operator () () const;

	inline bool operator == (const ptr&) const;
	inline bool operator == (ptr_type*) const;

	inline bool operator != (const ptr&) const;
	inline bool operator != (ptr_type*) const;

	inline bool operator >= (const ptr&) const;
	inline bool operator >= (ptr_type*) const;

	inline bool operator <= (const ptr&) const;
	inline bool operator <= (ptr_type*) const;

	inline bool operator > (const ptr&) const;
	inline bool operator > (ptr_type*) const;

	inline bool operator < (const ptr&) const;
	inline bool operator < (ptr_type*) const;

	~ptr();

protected:

	ptr_type* raw_ptr;

	inline void copy(const ptr&);
	inline void init(const ptr_type*);
	inline void cleanup();

	template <typename friend_type>
	friend struct ptr;

};

template <typename ptr_type>
template <typename cast_type>
inline ptr<cast_type> ptr<ptr_type>::cast() { return (cast_type*)raw_ptr; }

template <typename ptr_type>
ptr<ptr_type>::ptr() { init(nullptr); }

template <typename ptr_type>
ptr<ptr_type>::ptr(const ptr& ptr) { copy(ptr); }

template <typename ptr_type>
ptr<ptr_type>::ptr(const ptr_type* ptr) { init(ptr); }

template <typename ptr_type>
inline void ptr<ptr_type>::operator = (const ptr& ptr) { cleanup(); copy(ptr); }

template <typename ptr_type>
inline void ptr<ptr_type>::operator = (const ptr_type* ptr) { cleanup(); init(ptr); }

template <typename ptr_type>
inline ptr_type* ptr<ptr_type>::operator -> () const { return raw_ptr; }

template <typename ptr_type>
inline ptr_type& ptr<ptr_type>::operator * () const { return *raw_ptr; }

template <typename ptr_type>
inline ptr_type* ptr<ptr_type>::operator () () const { return raw_ptr; }

template <typename ptr_type>
inline bool ptr<ptr_type>::operator == (const ptr& rh) const { return raw_ptr == rh.raw_ptr; }

template <typename ptr_type>
inline bool ptr<ptr_type>::operator == (ptr_type* rh) const { return raw_ptr == rh; }

template <typename ptr_type>
inline bool ptr<ptr_type>::operator != (const ptr& rh) const { return raw_ptr != rh.raw_ptr; }

template <typename ptr_type>
inline bool ptr<ptr_type>::operator != (ptr_type* rh) const { return raw_ptr != rh; }

template <typename ptr_type>
inline bool ptr<ptr_type>::operator >= (const ptr& rh) const { return raw_ptr >= rh.raw_ptr; }

template <typename ptr_type>
inline bool ptr<ptr_type>::operator >= (ptr_type* rh) const { return raw_ptr >= rh; }

template <typename ptr_type>
inline bool ptr<ptr_type>::operator <= (const ptr& rh) const { return raw_ptr <= rh.raw_ptr; }

template <typename ptr_type>
inline bool ptr<ptr_type>::operator <= (ptr_type* rh) const { return raw_ptr <= rh; }

template <typename ptr_type>
inline bool ptr<ptr_type>::operator > (const ptr& rh) const { return raw_ptr > rh.raw_ptr; }

template <typename ptr_type>
inline bool ptr<ptr_type>::operator > (ptr_type* rh) const { return raw_ptr > rh; }

template <typename ptr_type>
inline bool ptr<ptr_type>::operator < (const ptr& rh) const { return raw_ptr < rh.raw_ptr; }

template <typename ptr_type>
inline bool ptr<ptr_type>::operator < (ptr_type* rh) const { return raw_ptr < rh; }

template <typename ptr_type>
inline ptr<ptr_type>::~ptr() { cleanup(); }

template <typename ptr_type>
inline void ptr<ptr_type>::copy(const ptr& ptr) { raw_ptr = ptr.raw_ptr; }

template <typename ptr_type>
inline void ptr<ptr_type>::init(const ptr_type* ptr) { raw_ptr = (ptr_type*)ptr; }

template <typename ptr_type>
inline void ptr<ptr_type>::cleanup() { }

#endif

#endif