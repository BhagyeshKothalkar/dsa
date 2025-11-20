#include <vector>
#include <algorithm>
#include <cassert>

class DSU {
public:
    // construct with n elements [0..n-1]
    explicit DSU(int n = 0) {
        reset(n);
    }

    // reset/initialize to n elements
    void reset(int n) {
        parent.assign(n, -1);
        rank.assign(n, 0);
        sz.assign(n, 1);
        for (int i = 0; i < n; ++i) parent[i] = i;
    }

    // create single element set for v (useful if you want to lazily add indices)
    void make_set(int v) {
        ensure_capacity(v);
        parent[v] = v;
        rank[v] = 0;
        sz[v] = 1;
    }

    // path-compressing find
    int find_set(int v) {
        assert(v >= 0 && v < (int)parent.size() && "find_set: index out of range");
        if (v == parent[v]) return v;
        return parent[v] = find_set(parent[v]);
    }

    // union by rank; merges sets containing a and b
    // returns true if union happened (were in different sets), false if already same set
    bool union_sets(int a, int b) {
        assert(a >= 0 && a < (int)parent.size() && b >= 0 && b < (int)parent.size()
               && "union_sets: index out of range");
        a = find_set(a);
        b = find_set(b);
        if (a == b) return false;

        if (rank[a] < rank[b]) std::swap(a, b);
        parent[b] = a;
        if (rank[a] == rank[b]) ++rank[a];

        // update size info
        sz[a] += sz[b];
        sz[b] = 0;
        return true;
    }

    // convenience: are two vertices in same set?
    bool connected(int a, int b) {
        assert(a >= 0 && a < (int)parent.size() && b >= 0 && b < (int)parent.size()
               && "connected: index out of range");
        return find_set(a) == find_set(b);
    }

    // size of component containing v
    int component_size(int v) {
        int root = find_set(v);
        return sz[root];
    }

    // number of elements currently managed
    int size() const { return static_cast<int>(parent.size()); }

private:
    std::vector<int> parent;
    std::vector<int> rank;
    std::vector<int> sz;

    // ensure internal arrays can index v; if not, resize and initialize new indices
    void ensure_capacity(int v) {
        if (v < (int)parent.size()) return;
        int old = static_cast<int>(parent.size());
        int newn = std::max(v + 1, old * 2);
        parent.resize(newn, -1);
        rank.resize(newn, 0);
        sz.resize(newn, 1);
        for (int i = old; i < newn; ++i) {
            parent[i] = i;
            rank[i] = 0;
            sz[i] = 1;
        }
    }
};
