#include <bits/stdc++.h>
using namespace std;

class Graph {
public:
    // adjacency: node -> vector of (neighbor, weight)
    explicit Graph(int n = 0) : adj(n) {}

    void resize(int n) { adj.assign(n, {}); }

    void addEdge(int u, int v, int w = 1) {            
        check_index(u); check_index(v);
        adj[u].emplace_back(v, w);
    }

    void addUndirectedEdge(int u, int v, int w = 1) {  
        check_index(u); check_index(v);
        adj[u].emplace_back(v, w);
        adj[v].emplace_back(u, w);
    }

    void dfs(int start) const {
        check_index(start);
        vector<char> vis(adj.size(), 0);
        vector<int> st;
        st.reserve(adj.size());
        st.push_back(start);
        while (!st.empty()) {
            int u = st.back();
            st.pop_back();
            if (vis[u]) continue;
            vis[u] = 1;
            cout << u << " ";
            for (auto &p : adj[u]) {
                int v = p.first;
                if (!vis[v]) st.push_back(v);
            }
        }
        cout << "\n";
    }

    pair<long long, vector<int>> dijkstra(int src, int tgt) const {
        check_index(src);
        check_index(tgt);
        const long long INF = (1LL << 60);
        int n = (int)adj.size();
        vector<long long> dist(n, INF);
        vector<int> parent(n, -1);
        using P = pair<long long,int>;
        priority_queue<P, vector<P>, greater<P>> pq;

        dist[src] = 0;
        pq.push({0, src});

        while (!pq.empty()) {
            auto [d, u] = pq.top(); pq.pop();
            if (d != dist[u]) continue;
            if (u == tgt) break; // early exit safe in Dijkstra
            for (auto &e : adj[u]) {
                int v = e.first;
                long long w = e.second;
                if (dist[v] > d + w) {
                    dist[v] = d + w;
                    parent[v] = u;
                    pq.push({dist[v], v});
                }
            }
        }

        vector<int> path;
        if (dist[tgt] == INF) return {INF, path};
        for (int v = tgt; v != -1; v = parent[v]) path.push_back(v);
        reverse(path.begin(), path.end());
        return {dist[tgt], path};
    }

    vector<vector<pair<int,int>>> primMST() const {
        int n = (int)adj.size();
        vector<vector<pair<int,int>>> mst(n);
        if (n == 0) return mst;

        const long long INF = (1LL<<60);
        vector<char> used(n, 0);

        for (int start = 0; start < n; ++start) {
            if (used[start]) continue;

            // Min-heap of (weight, from, to)
            using Item = tuple<int,int,int>;
            priority_queue<Item, vector<Item>, greater<Item>> pq;
            used[start] = 1;
            for (auto &e : adj[start]) {
                if (!used[e.first]) pq.emplace(e.second, start, e.first);
            }

            while (!pq.empty()) {
                auto [w, u, v] = pq.top(); pq.pop();
                if (used[v]) continue;
                used[v] = 1;
                mst[u].emplace_back(v, w);
                mst[v].emplace_back(u, w);
                for (auto &e : adj[v]) {
                    if (!used[e.first]) pq.emplace(e.second, v, e.first);
                }
            }
        }
        return mst;
    }

    vector<int> topoSort() const {
        int n = (int)adj.size();
        vector<int> indeg(n, 0);
        for (int u = 0; u < n; ++u)
            for (auto &e : adj[u]) indeg[e.first]++;

        queue<int> q;
        for (int i = 0; i < n; ++i) if (indeg[i] == 0) q.push(i);

        vector<int> order;
        order.reserve(n);
        while (!q.empty()) {
            int u = q.front(); q.pop();
            order.push_back(u);
            for (auto &e : adj[u]) {
                int v = e.first;
                if (--indeg[v] == 0) q.push(v);
            }
        }
        if ((int)order.size() != n) return {}; // cycle detected
        return order;
    }

    vector<int> findCycleDirected() const {
        int n = (int)adj.size();
        vector<int> color(n, 0);
        vector<int> parent(n, -1);
        vector<int> cycle;

        function<bool(int)> dfs = [&](int u)->bool {
            color[u] = 1;
            for (auto &e : adj[u]) {
                int v = e.first;
                if (color[v] == 0) {
                    parent[v] = u;
                    if (dfs(v)) return true;
                } else if (color[v] == 1) {
                    int cur = u;
                    cycle.push_back(v);
                    while (cur != v) { cycle.push_back(cur); cur = parent[cur]; }
                    reverse(cycle.begin(), cycle.end());
                    return true;
                }
            }
            color[u] = 2;
            return false;
        };

        for (int i = 0; i < n; ++i) {
            if (color[i] == 0) {
                if (dfs(i)) return cycle;
            }
        }
        return {}; 
    }

    vector<int> findCycleUndirected() const {
        int n = (int)adj.size();
        vector<char> vis(n, 0);
        vector<int> parent(n, -1);
        vector<int> cycle;

        function<bool(int,int)> dfs = [&](int u, int p)->bool {
            vis[u] = 1;
            for (auto &e : adj[u]) {
                int v = e.first;
                if (v == p) continue;
                if (!vis[v]) {
                    parent[v] = u;
                    if (dfs(v, u)) return true;
                } else {
                    // found cycle u--v, reconstruct
                    int cur = u;
                    cycle.push_back(v);
                    while (cur != v) { cycle.push_back(cur); cur = parent[cur]; }
                    cycle.push_back(v);
                    reverse(cycle.begin(), cycle.end());
                    return true;
                }
            }
            return false;
        };

        for (int i = 0; i < n; ++i) {
            if (!vis[i]) {
                if (dfs(i, -1)) return cycle;
            }
        }
        return {};
    }

    bool isBipartite() const {
        int n = (int)adj.size();
        vector<int> color(n, -1);
        queue<int> q;

        for (int s = 0; s < n; ++s) {
            if (color[s] != -1) continue;
            color[s] = 0;
            q.push(s);
            while (!q.empty()) {
                int u = q.front(); q.pop();
                for (auto &e : adj[u]) {
                    int v = e.first;
                    if (color[v] == -1) {
                        color[v] = color[u] ^ 1;
                        q.push(v);
                    } else if (color[v] == color[u]) {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    int vertexCount() const { return (int)adj.size(); }
    int edgeCount() const {
        long long cnt = 0;
        for (auto &nbrs : adj) cnt += (int)nbrs.size();
        return (int)cnt;
    }

private:
    vector<vector<pair<int,int>>> adj;

    void check_index(int v) const {
        if (v < 0 || v >= (int)adj.size()) {
            throw out_of_range("vertex index out of range");
        }
    }
};

int main() {
    cout << "--- Testing Cyclic Undirected Graph ---\n";
    {
        Graph g(4);
        g.addUndirectedEdge(0,1,4);
        g.addUndirectedEdge(1,2,3);
        g.addUndirectedEdge(2,0,2);
        g.addUndirectedEdge(2,3,5);

        auto cycle = g.findCycleUndirected();
        if (cycle.empty()) cout << "Acyclic\n";
        else {
            cout << "Found undirected cycle: ";
            for (int v: cycle) cout << v << " ";
            cout << "\n";
        }

        cout << "Is Bipartite? " << (g.isBipartite() ? "Yes" : "No") << "\n";

        auto [dist, path] = g.dijkstra(0, 3);
        if (dist >= (1LL<<59)) cout << "No path\n";
        else {
            cout << "Dijkstra path 0->3 (dist="<<dist<<"): ";
            for (int v: path) cout << v << " ";
            cout << "\n";
        }
    }

    cout << "\n--- Testing DAG ---\n";
    {
        Graph g(5);
        g.addEdge(0,1,2);
        g.addEdge(0,2,1);
        g.addEdge(1,3,4);
        g.addEdge(2,3,2);
        g.addEdge(3,4,1);

        auto topo = g.topoSort();
        if (topo.empty()) cout << "Graph has a directed cycle; no topo order\n";
        else {
            cout << "Toposort: ";
            for (int v : topo) cout << v << " ";
            cout << "\n";
        }

        auto cycle = g.findCycleDirected();
        if (cycle.empty()) cout << "Directed graph: Acyclic\n";
        else {
            cout << "Directed cycle: ";
            for (int v: cycle) cout << v << " ";
            cout << "\n";
        }

        auto mst = g.primMST();
        cout << "MST adjacency (per-component):\n";
        for (int u = 0; u < g.vertexCount(); ++u) {
            if (!mst[u].empty()) {
                cout << u << ": ";
                for (auto &e : mst[u]) cout << "(" << e.first << "," << e.second << ") ";
                cout << "\n";
            }
        }
    }

    return 0;
}
