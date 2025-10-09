#include <iostream>
#include <stdexcept>
#include <vector>

using namespace std;

template <typename T>
void merge(vector<T> &vec, size_t l1, size_t r1, size_t l2, size_t r2) {

    if (!(l1 <= r1 && l2 <= r2))
        throw invalid_argument("the value of l should be less than r for both "
                               "arrays");

    size_t i = 0, j = 0, maxj = r2 + 1 - l2, maxi = r1 + 1 - l1;
    vector<T> aux(maxi + maxj);
    while (i < maxi && j < maxj) {
        if (vec[l1 + i] < vec[l2 + j])
            aux[i + j] = vec[l1 + i], i++;
        else
            aux[i + j] = vec[l2 + j], j++;
    }
    if (i == maxi) {
        while (j < maxj)
            aux[i + j] = vec[l2 + j];
    } else {
        while (i < maxi)
            aux[i + j] = vec[l1 + i];
    }
    for (size_t i = 0; i < maxi; i++) {
        vec[l1 + i] = aux[i];
    }
    for (size_t j = 0; j < maxj; j++) {
        vec[l2 + j] = aux[maxi + j];
    }
}

template <typename T>
void mergesort(vector<T> &vec, size_t l = 0, size_t r = 0) {
    if (vec.empty())
        return;
    if (r == 0)
        r = vec.size() - 1;
    if (l >= r)
        return;
    size_t m = (l + r) / 2;
    mergesort(vec, l, m);
    mergesort(vec, m + 1, r);
    merge(vec, l, m, m + 1, r);
}
