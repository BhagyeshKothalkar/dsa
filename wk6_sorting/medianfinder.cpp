#include <iostream>
using namespace std;

class medianfinder {
private:
  int *arr;
  int sz;

  void swap(int &a, int &b) { int t = a, a = b, b = t; }

public:
  medianfinder(int max_size) : sz(0) { arr = new int[max_size]; }
  void add_elem(int k) {
    arr[sz] = k;
    int t = sz;
    sz++;
    while (t > 0) {
      if (arr[t] < arr[t - 1])
        swap(arr[t], arr[t - 1]); // O(n)
    }
  }
  int getmedian() {
    if (sz % 2)
      return arr[(sz + 1) / 2];
    return (arr[(sz + 1) / 2] + arr[sz / 2])/2; // O(1)
  }
};

class medianfinder {
private:
  int *arr;
  int sz;

  void swap(int &a, int &b) { int t = a, a = b, b = t; }

public:
  medianfinder(int max_size) : sz(0) { arr = new int[max_size]; }
  void add_elem(int k) {
    arr[sz] = k;
    int t = sz;
    sz++;
    while (t > 0) {
      if (arr[t] < arr[t - 1])
        swap(arr[t], arr[t - 1]); // O(n)
    }
  }
  int getmedian() {
    if (sz % 2)
      return arr[(sz + 1) / 2];
    return (arr[(sz + 1) / 2] + arr[sz / 2])/2; // O(1)
  }
};