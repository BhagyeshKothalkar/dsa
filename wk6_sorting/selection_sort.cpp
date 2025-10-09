#include <iostream>
#include <vector>

using namespace std;

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
