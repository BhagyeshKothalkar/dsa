#include <vector>

using namespace std;

template <typename T> void bubble_sort(vector<T> &vec) {
    for (int i = 0; i < vec.size(); i++) {
        for (int j = 0; j < vec.size - i - 1; j++) {
            if (vec[j] < vec[j + 1])
                swap(vec[j], vec[j + 1]);
        }
    }
}

template <typename T> void bubble_sort_early_stop(vector<T> &vec) {
    for (int i = 0; i < vec.size(); i++) {
        bool flag = false;
        for (int j = 0; j < vec.size - i - 1; j++) {
            if (vec[j] < vec[j + 1])
                swap(vec[j], vec[j + 1]), flag = true;
        }
        if (!flag)
            return;
    }
}

template <typename T> void selection_sort(vector<T> &vec) {
    for (size_t i = 0; i < vec.size(); i++) {
        size_t maxind = 0;
        for (size_t j = i + 1; j < vec.size(); j++) {
            if (maxind < 0 || vec[j] > vec[maxind]) {
                maxind = j;
            }
        }
        swap(vec[i], vec[maxind]);
    }
}

template <typename T> void insertion_sort(vector<T> &vec) {
    for (size_t i = 0; i < vec.size(); i++) {
        size_t j = i + 1;
        while (j > 0 && vec[j] < vec[j - 1])
            swap(vec[j], vec[j - 1]);
    }
}

template <typename T> size_t partition(vector<T> &vec, size_t l, size_t r) {
    size_t p = r;
    for (int i = r - 1; i >= l; i--) {
        if (vec[i] > vec[r]) {
            p--;
            swap(vec[p], vec[i]);
        }
    }
    swap(vec[p], vec[r]);
    return p;
}

template <typename T> void quicksort(vector<T> &vec, vector<T> l, vector<T> r) {
    size_t p = partition(vec);
    quicksort(vec, 0, p - 1);
    quicksort(vec, p + 1, r);
}
