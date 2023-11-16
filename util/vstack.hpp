#ifndef VSTACK_HPP
#define VSTACK_HPP

#include "./vec.hpp"

template <typename T>
struct vstack {

	vstack() { };

	virtual void clear() { v.clear(); top_idx = 0; }

	void push(T x) { if (top_idx == v.size()) v.push_back(x); else v[top_idx] = x; top_idx++; }

	void pop() { if (top_idx > 0) top_idx--; v.resize(top_idx); }

	virtual T& top() { return v[top_idx - 1]; }

	T& operator [] (int idx) { return v[idx]; }

	managed_vec<T>& operator () () { return v; }

	int size() const { return top_idx; }

protected:

	managed_vec<T> v;
	int top_idx = 0;

};

#endif