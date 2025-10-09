// #include<iostream>
// #include<climits> // for int min when we find max

using namespace std;

const int n = 1000;
int heap[n], sz = 0;

void swap(int &a, int &b) { int t = a, a = b, b = t; }

void insert(int val) {
  heap[++sz] = val;
  int i = sz;
  while (i > 1) {
    if (heap[i / 2] < heap[i])
      swap(heap[i / 2], heap[i]);
    else
      break;
  }
}

void hippyphai(int idx) {
  while (true) {
    int mx = idx;
    if (2 * idx + 1 <= sz && heap[mx] < heap[2 * idx + 1])
      mx = 2 * idx + 1;
    if (2 * idx <= sz && heap[mx] < heap[2 * idx])
      mx = 2 * idx;
    if (mx != idx)
      swap(heap[mx], heap[idx]), idx = mx;
    else
      break;
  }
}

void heapify(int n) {
  int mx = n;
  if (2 * n + 1 <= sz && heap[mx] < heap[2 * n + 1]) {
    mx = 2 * n + 1;
  }
  if (2 * n <= sz && heap[mx] < heap[2 * n]) {
    mx = 2 * n;
  }
  if (mx == n)
    return;
  swap(heap[n], heap[mx]);
  heapify(mx);
}

void del(int idx) {
  swap(heap[idx], heap[sz--]);
  heapify(idx);
}

void make_heap() {
  for (int i = sz; i >= 1; i--)
    heapify(i);
}