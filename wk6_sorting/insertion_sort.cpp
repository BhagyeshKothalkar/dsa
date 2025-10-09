#include <iostream>
#include <vector>

using namespace std;

template <typename T> void insertion_sort(vector<T> &vec) {
    for (size_t i = 0; i < vec.size(); i++) {
        size_t j = i + 1;
        while (j > 0 && vec[j] < vec[j - 1])
            swap(vec[j], vec[j - 1]);
    }
}
