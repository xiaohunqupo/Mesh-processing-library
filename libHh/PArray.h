// -*- C++ -*-  Copyright (c) Microsoft Corporation; see license.txt
#ifndef MESH_PROCESSING_LIBHH_PARRAY_H_
#define MESH_PROCESSING_LIBHH_PARRAY_H_

#include "libHh/Array.h"  // ArrayView

namespace hh {

// Resizable allocated 1D array just like Array<T> but contains built-in storage for pcap elements.
//  As in Array<T>, T must have a public operator=().
template <typename T, int pcap> class PArray : public ArrayView<T> {  // Pre-allocated capacity pcap.
  using base = ArrayView<T>;
  using type = PArray<T, pcap>;

 public:
  PArray() : base(_pa, 0) {}
  explicit PArray(int n) : base(_pa, n) {
    ASSERTX(n >= 0);
    if (n > pcap) {
      _a = new T[narrow_cast<size_t>(n)];
      _cap = n;
    }
  }
  explicit PArray(const type& ar) : PArray() { *this = ar; }
  explicit PArray(CArrayView<T> ar) : PArray() { *this = ar; }
  PArray(type&& ar) : PArray() { *this = std::move(ar); }
  PArray(std::initializer_list<T> l) : PArray(CArrayView<T>(l)) {}
  template <typename I> explicit PArray(I b, I e) : PArray() {
    for (; b != e; ++b) push(*b);
  }
  template <typename Range, typename = enable_if_range_t<Range>>
  explicit PArray(Range&& range) : PArray(range.begin(), range.end()) {}
  ~PArray() {
    if (_a != _pa) delete[] _a;  // Equivalent to "if (_cap != pcap)".
  }
  auto& operator=(CArrayView<T> ar) {
    if (!(ar.data() == _a && ar.num() == _n)) {
      init(ar.num());
      std::copy(ar.begin(), ar.end(), _a);
    }
    return *this;
  }
  auto& operator=(const type& ar) {
    if (&ar != this) {
      init(ar.num());
      std::copy(ar.begin(), ar.end(), _a);
    }
    return *this;
  }
  auto& operator=(type&& ar) noexcept {
    clear();
    if (ar._cap != pcap) {
      std::swap(_a, ar._a);
      std::swap(_n, ar._n);
      std::swap(_cap, ar._cap);
      ar._a = ar._pa;
    } else {
      std::move(ar._pa, ar._pa + ar._n, _pa);
      std::swap(_n, ar._n);
    }
    return *this;
  }
  void clear() noexcept {
    _n = 0;
    if (_a != _pa) {
      delete[] _a;
      _a = _pa;
      _cap = pcap;
    }
  }
  void init(int n) {  // Allocate n, DISCARD old values if too small.
    ASSERTX(n >= 0);
    if (n > _cap) {
      if (_a != _pa) delete[] _a;
      _a = new T[narrow_cast<size_t>(n)];
      _cap = n;
    }
    _n = n;
  }
  void resize(int n) {  // Allocate n, RETAIN old values (using move if too small).
    ASSERTX(n >= 0);
    if (n > _cap) grow_to_at_least(n);
    _n = n;
  }
  void access(int i) {  // Allocate at least i + 1, RETAIN old values (using move if too small).
    ASSERTX(i >= 0);
    int n = i + 1;
    if (n > _cap) grow_to_at_least(n);
    if (n > _n) _n = n;
  }
  int add(int n) {
    ASSERTX(n >= 0);
    int cn = _n;
    resize(_n + n);
    return cn;
  }
  void sub(int n) {
    ASSERTX(n >= 0);
    _n -= n;
    ASSERTX(_n >= 0);
  }
  void shrink_to_fit() {
    if (_n < _cap) set_capacity(_n);
  }
  void reserve(int s) {
    ASSERTX(s >= 0);
    if (_cap < s) set_capacity(s);
  }
  int capacity() const { return _cap; }
  void insert(int i, int n) { ASSERTX(i >= 0 && i <= _n), insert_i(i, n); }
  void erase(int i, int n) { ASSERTX(i >= 0 && n >= 0 && i + n <= _n), erase_i(i, n); }
  bool remove_ordered(const T& e) {  // Return: was there.
    for_int(i, _n) {
      if (_a[i] == e) {
        erase(i, 1);
        return true;
      }
    }
    return false;
  }
  bool remove_unordered(const T& e) {  // Return: was there.
    for_int(i, _n) {
      if (_a[i] == e) {
        if (i < _n - 1) _a[i] = std::move(_a[_n - 1]);
        sub(1);
        return true;
      }
    }
    return false;
  }
  T pop() {
    ASSERTX(_n);
    T e = std::move(_a[_n - 1]);
    sub(1);
    return e;
  }
  void push(const T& e) {  // Avoid a.push(a[..])!
    if (_n >= _cap) grow_to_at_least(_n + 1);
    _a[_n++] = e;
  }
  void push(T&& e) {
    if (_n >= _cap) grow_to_at_least(_n + 1);
    _a[_n++] = std::move(e);
  }
  void push(CArrayView<T> ar) {
    int n = ar.num();
    add(n);
    for_int(i, n) _a[_n - n + i] = ar[i];
  }
  void push(type&& ar) {
    int n = ar.num();
    add(n);
    for_int(i, n) _a[_n - n + i] = std::move(ar[i]);
  }
  T shift() {
    ASSERTX(_n);
    T e = std::move(_a[0]);
    erase_i(0, 1);
    return e;
  }
  void unshift(const T& e) { insert_i(0, 1), _a[0] = e; }
  void unshift(T&& e) { insert_i(0, 1), _a[0] = std::move(e); }
  void unshift(CArrayView<T> ar) {
    int n = ar.num();
    insert_i(0, n);
    for_int(i, n) _a[i] = ar[i];
  }
  void unshift(type&& ar) {
    int n = ar.num();
    insert_i(0, n);
    for_int(i, n) _a[i] = std::move(ar[i]);
  }
  friend void swap(type& l, type& r) noexcept {
    if (l._cap > pcap) {
      if (r._cap > pcap) {
        std::swap(l._a, r._a);
        std::swap(l._cap, r._cap);
        std::swap(l._n, r._n);
        // Contents of _pa remains undefined in both.
      } else {
        std::move(r._pa, r._pa + r._n, l._pa);
        r._a = l._a;
        l._a = l._pa;
        std::swap(l._n, r._n);
        std::swap(l._cap, r._cap);
      }
    } else {
      if (r._cap > pcap) {
        std::move(l._pa, l._pa + l._n, r._pa);
        l._a = r._a;
        r._a = r._pa;
        std::swap(l._n, r._n);
        std::swap(l._cap, r._cap);
      } else {
        std::swap_ranges(l._pa, l._pa + max(l._n, r._n), r._pa);
        std::swap(l._n, r._n);
        // Fields _a and _cap are unchanged in both.
      }
    }
  }

 private:
  using base::_a;
  using base::_n;
  using base::reinit;  // Hide it.
  int _cap{pcap};
  T _pa[pcap];
  void set_capacity(int ncap) {
    ASSERTX(_n <= ncap);
    if (ncap <= pcap) {
      if (_a == _pa) return;
      std::move(_a, _a + _n, _pa);
      delete[] _a;
      _a = _pa;
      _cap = pcap;
    } else {
      T* na = new T[narrow_cast<size_t>(ncap)];
      std::move(_a, _a + _n, na);
      if (_a != _pa) delete[] _a;
      _a = na;
      _cap = ncap;
    }
  }
  void grow_to_at_least(int n) { set_capacity(max(_n + (_n / 2) + 3, n)); }
  void insert_i(int i, int n) {
    add(n);
    for (int j = _n - n - 1; j >= i; --j) _a[j + n] = std::move(_a[j]);
  }
  void erase_i(int i, int n) {
    for_intL(j, i, _n - n) _a[j] = std::move(_a[j + n]);
    sub(n);
  }
  void ok() const {
    if (_cap > pcap) {
      ASSERTX(_a);
      ASSERTX(_a != _pa);
    } else {
      ASSERTX(_cap == pcap);
      ASSERTX(_a == _pa);
    }
    ASSERTX(_n <= _cap);
  }
  type& operator+=(const T&) = delete;  // Dangerous because ambiguous (push() or add to all elements).
};

// Given container c, evaluate func() on each element (possibly changing the element type) and return new container.
template <typename T, int pcap, typename Func> auto map(const PArray<T, pcap>& c, Func func) {
  PArray<decltype(func(std::declval<T>())), pcap> nc(c.num());
  for_int(i, c.num()) nc[i] = func(c[i]);
  return nc;
}

template <typename T, int pcap> HH_DECLARE_OSTREAM_EOL(PArray<T, pcap>);  // Implemented by CArrayView<T>.

}  // namespace hh

#endif  // MESH_PROCESSING_LIBHH_PARRAY_H_
