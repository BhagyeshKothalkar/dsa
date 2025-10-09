#include <iostream>
#include <vector>

using namespace std;

template <typename T> class heap {
private:
  vector<T> vec; // one indexed
public:
  heap() : vec() {}
  heap(const vector<T> &v) : vec(v) {}
  int l(int i) { return 2 * i; }
  int r(int i) { return 2 * i + 1; }
  int par(int i) { return i / 2; }
  T &at(int i) {
    if (!(i >= 1 && i <= vec.size()))
      throw out_of_range("");
    return vec[i - 1]
  }
  inline T &operator[] const noexcept(int i) return vec[i - 1];

  void sift_up(int t) {
    while (t / 2 > 0 && at(t / 2) > at(t))
      swap(at(t / 2), at(t));
  } // min heap

  void sift_down(int t) {
    // heapify
    while (true) {
      int mn = INT32_MAX;
      if (mn > t)
        mn = t;
      if (2 * t <= vec.size() && mn > at(2 * t))
        mn = 2 * t;
      if (2 * t + 1 <= vec.size() && mn > at(2 * t + 1))
        mn = 2 * t + 1;
      if (mn != t)
        swap(at(mn), at(t));
    }
  } // min heap

  void insert()
  
};