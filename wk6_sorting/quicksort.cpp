#include <iostream>
#include <vector>

using namespace std;

template <typename T>
size_t partition(vector<T> &vec, size_t l, size_t r, size_t pivot) {
    // Move pivot to end
    swap(vec[pivot], vec[r]);
    T pivotValue = vec[r];
    size_t store = l;
    for (size_t i = l; i < r; ++i) {
        if (vec[i] < pivotValue) {
            swap(vec[i], vec[store]);
            ++store;
        }
    }
    // Move pivot to its final place
    swap(vec[store], vec[r]);
    return store;
}

template <typename T> void quicksort(vector<T> &vec, size_t l, size_t r) {
    if (vec.empty())
        return;
    if (l >= r)
        return;
    size_t p = partition(vec, l, r, r);
    quicksort(vec, l, p - 1);
    quicksort(vec, p + 1, r);
}
