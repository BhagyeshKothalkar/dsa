#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
#include <optional>
#include <cassert>

/*
  Canonical 2-3-4 tree (B-tree with minimum degree t = 2).
  Template parameters:
    Key     - key type
    Compare - comparator (default std::less<Key>)

  Public API:
    insert(const Key&), remove(const Key&), contains(const Key&), size(), clear(),
    inorder_traversal(fn), batch_load(vec, deduplicate)
*/

template <typename Key, typename Compare = std::less<Key>>
class BTree234 {
public:
    BTree234() : root(nullptr), tree_size(0), comp(Compare()) {}
    explicit BTree234(Compare cmp) : root(nullptr), tree_size(0), comp(cmp) {}
    ~BTree234() { clear(); }

    // Public API
    bool insert(const Key& key) {
        if (!root) {
            root = new Node(true);
            root->keys[0] = key;
            root->n = 1;
            ++tree_size;
            return true;
        }

        // If root is full, split and create new root
        if (root->n == MAX_KEYS) {
            Node* s = new Node(false);
            s->children[0] = root;
            split_child(s, 0);
            root = s;
        }

        bool inserted = insert_nonfull(root, key);
        if (inserted) ++tree_size;
        return inserted;
    }

    bool remove(const Key& key) {
        if (!root) return false;
        bool removed = remove_internal(root, key);
        if (removed) {
            --tree_size;
            // If root became empty and has a child, make that child the new root
            if (root->n == 0) {
                Node* old = root;
                if (!root->leaf) root = root->children[0];
                else root = nullptr;
                delete old;
            }
        }
        return removed;
    }

    bool contains(const Key& key) const {
        return search(root, key) != nullptr;
    }

    std::size_t size() const { return tree_size; }

    void clear() {
        clear_rec(root);
        root = nullptr;
        tree_size = 0;
    }

    void inorder_traversal(const std::function<void(const Key&)>& fn) const {
        inorder_rec(root, fn);
    }

    // Batch load: deduplicate optionally (sort+unique) then insert sequentially.
    void batch_load(const std::vector<Key>& items, bool deduplicate = true) {
        if (items.empty()) return;
        if (deduplicate) {
            std::vector<Key> tmp = items;
            std::sort(tmp.begin(), tmp.end(), comp);
            tmp.erase(std::unique(tmp.begin(), tmp.end(),
                                  [&](const Key& a, const Key& b){ return !comp(a,b) && !comp(b,a); }),
                      tmp.end());
            clear();
            for (const auto& v : tmp) insert(v);
        } else {
            for (const auto& v : items) insert(v);
        }
    }

    // Debugger: print tree structure
    void debug_print() const {
        debug_print_rec(root, 0);
    }

    // disable copy
    BTree234(const BTree234&) = delete;
    BTree234& operator=(const BTree234&) = delete;

private:
    // canonical minimum degree for 2-3-4 tree
    static constexpr int MIN_DEGREE = 2;                 // t = 2
    static constexpr int MAX_KEYS = 2 * MIN_DEGREE - 1;  // 3
    static constexpr int MAX_CHILDREN = 2 * MIN_DEGREE;  // 4
    static constexpr int MIN_KEYS = MIN_DEGREE - 1;      // 1

    struct Node {
        bool leaf;
        int n; // number of keys
        Key keys[MAX_KEYS];
        Node* children[MAX_CHILDREN];

        explicit Node(bool leaf_ = true) : leaf(leaf_), n(0) {
            for (int i = 0; i < MAX_CHILDREN; ++i) children[i] = nullptr;
        }
    };

    Node* root;
    std::size_t tree_size;
    Compare comp;

    // -------------------------
    // Utility (static where possible)
    // -------------------------
    static void clear_rec(Node* node) {
        if (!node) return;
        for (int i = 0; i <= node->n; ++i) clear_rec(node->children[i]);
        delete node;
    }

    static void inorder_rec(Node* node, const std::function<void(const Key&)>& fn) {
        if (!node) return;
        for (int i = 0; i < node->n; ++i) {
            inorder_rec(node->children[i], fn);
            fn(node->keys[i]);
        }
        inorder_rec(node->children[node->n], fn);
    }

    static void debug_print_rec(Node* node, int depth) {
        if (!node) return;
        for (int i = 0; i < depth; ++i) std::cout << "  ";
        std::cout << "[";
        for (int i = 0; i < node->n; ++i) {
            std::cout << node->keys[i];
            if (i + 1 < node->n) std::cout << "|";
        }
        std::cout << "]";
        std::cout << (node->leaf ? " (L)\n" : "\n");
        if (!node->leaf) {
            for (int i = 0; i <= node->n; ++i) debug_print_rec(node->children[i], depth + 1);
        }
    }

    // -------------------------
    // Search (non-static because uses comp)
    // -------------------------
    Node* search(Node* node, const Key& key) const {
        if (!node) return nullptr;
        int i = 0;
        while (i < node->n && comp(node->keys[i], key)) ++i;
        if (i < node->n && !comp(key, node->keys[i]) && !comp(node->keys[i], key)) {
            return node;
        }
        if (node->leaf) return nullptr;
        return search(node->children[i], key);
    }

    // -------------------------
    // Insertion helpers
    // -------------------------
    bool insert_nonfull(Node* node, const Key& key) {
        int i = node->n - 1;
        if (node->leaf) {
            // locate insert pos
            while (i >= 0 && comp(key, node->keys[i])) {
                node->keys[i + 1] = node->keys[i];
                --i;
            }
            if (i >= 0 && !comp(key, node->keys[i]) && !comp(node->keys[i], key)) {
                return false; // duplicate
            }
            node->keys[i + 1] = key;
            node->n += 1;
            return true;
        } else {
            while (i >= 0 && comp(key, node->keys[i])) --i;
            ++i; // child index
            // child is full => split
            if (node->children[i]->n == MAX_KEYS) {
                split_child(node, i);
                // after split, decide where to go
                if (comp(node->keys[i], key)) ++i;
                else if (!comp(key, node->keys[i]) && !comp(node->keys[i], key)) {
                    return false; // duplicate equal to promoted key
                }
            }
            return insert_nonfull(node->children[i], key);
        }
    }

    void split_child(Node* parent, int idx) {
        Node* y = parent->children[idx];
        assert(y && y->n == MAX_KEYS);
        Node* z = new Node(y->leaf);

        // z receives right half keys of y
        z->n = MIN_DEGREE - 1; // t-1 = 1 for t=2
        for (int j = 0; j < MIN_DEGREE - 1; ++j) {
            z->keys[j] = y->keys[j + MIN_DEGREE];
        }

        // copy children if internal
        if (!y->leaf) {
            for (int j = 0; j < MIN_DEGREE; ++j) {
                z->children[j] = y->children[j + MIN_DEGREE];
            }
        }

        // reduce y
        y->n = MIN_DEGREE - 1;

        // shift parent's children and insert z
        for (int j = parent->n; j >= idx + 1; --j) parent->children[j + 1] = parent->children[j];
        parent->children[idx + 1] = z;

        // shift parent's keys and move middle key up
        for (int j = parent->n - 1; j >= idx; --j) parent->keys[j + 1] = parent->keys[j];
        parent->keys[idx] = y->keys[MIN_DEGREE - 1];
        parent->n += 1;
    }

    // -------------------------
    // Borrow / merge / rotate helpers
    // -------------------------
    void borrow_from_prev(Node* parent, int idx) {
        Node* child = parent->children[idx];
        Node* left = parent->children[idx - 1];
        assert(child && left);

        // shift child's keys right
        for (int i = child->n - 1; i >= 0; --i) child->keys[i + 1] = child->keys[i];
        if (!child->leaf) {
            for (int i = child->n; i >= 0; --i) child->children[i + 1] = child->children[i];
        }

        // move parent's key down to child
        child->keys[0] = parent->keys[idx - 1];

        // move left's last key up to parent
        parent->keys[idx - 1] = left->keys[left->n - 1];

        if (!left->leaf) {
            child->children[0] = left->children[left->n];
        }

        left->n -= 1;
        child->n += 1;
    }

    void borrow_from_next(Node* parent, int idx) {
        Node* child = parent->children[idx];
        Node* right = parent->children[idx + 1];
        assert(child && right);

        // put parent's key at child's end
        child->keys[child->n] = parent->keys[idx];

        if (!child->leaf) {
            child->children[child->n + 1] = right->children[0];
        }

        // move right's first key up
        parent->keys[idx] = right->keys[0];

        // shift right's keys/children left
        for (int i = 0; i < right->n - 1; ++i) right->keys[i] = right->keys[i + 1];
        if (!right->leaf) {
            for (int i = 0; i < right->n; ++i) right->children[i] = right->children[i + 1];
        }

        right->n -= 1;
        child->n += 1;
    }

    void merge_children(Node* parent, int idx) {
        Node* left = parent->children[idx];
        Node* right = parent->children[idx + 1];
        assert(left && right);

        // pull down parent key
        left->keys[MIN_DEGREE - 1] = parent->keys[idx];

        // copy keys from right into left
        for (int i = 0; i < right->n; ++i) left->keys[i + MIN_DEGREE] = right->keys[i];

        // copy children
        if (!left->leaf) {
            for (int i = 0; i <= right->n; ++i) left->children[i + MIN_DEGREE] = right->children[i];
        }

        // shift parent's keys/children left to fill gap
        for (int i = idx + 1; i < parent->n; ++i) parent->keys[i - 1] = parent->keys[i];
        for (int i = idx + 2; i <= parent->n; ++i) parent->children[i - 1] = parent->children[i];

        left->n += right->n + 1;
        parent->n -= 1;

        delete right;
    }

    // conceptual rotate wrappers (map to borrow/merge)
    void rotate_left(Node* parent, int child_idx) {
        // borrow from next if possible, else merge
        Node* child = parent->children[child_idx];
        Node* r = parent->children[child_idx + 1];
        if (r->n >= MIN_DEGREE) borrow_from_next(parent, child_idx);
        else merge_children(parent, child_idx);
    }

    void rotate_right(Node* parent, int child_idx) {
        Node* child = parent->children[child_idx];
        Node* l = parent->children[child_idx - 1];
        if (l->n >= MIN_DEGREE) borrow_from_prev(parent, child_idx);
        else merge_children(parent, child_idx - 1);
    }

    // -------------------------
    // Deletion helpers (canonical)
    // -------------------------
    bool remove_internal(Node* node, const Key& key) {
        if (!node) return false;
        int idx = find_key_index(node, key);

        // Key found in this node
        if (idx < node->n && !comp(key, node->keys[idx]) && !comp(node->keys[idx], key)) {
            if (node->leaf) {
                // Case 1: node is leaf -> remove directly
                for (int i = idx + 1; i < node->n; ++i) node->keys[i - 1] = node->keys[i];
                node->n -= 1;
                return true;
            } else {
                // Case 2: key in internal node
                return remove_from_nonleaf(node, idx);
            }
        } else {
            // Key not present in this node
            if (node->leaf) return false; // not found

            bool last_child = (idx == node->n);
            if (node->children[idx]->n < MIN_DEGREE) {
                fill(node, idx);
            }
            // After fill, the child pointers may have changed.
            if (last_child && idx > node->n) {
                // we merged the last two children; recurse on idx-1
                return remove_internal(node->children[idx - 1], key);
            } else {
                return remove_internal(node->children[idx], key);
            }
        }
    }

    bool remove_from_nonleaf(Node* node, int idx) {
        Key k = node->keys[idx];
        // If left child has at least t keys, replace k by predecessor
        if (node->children[idx]->n >= MIN_DEGREE) {
            Key pred = get_predecessor(node, idx);
            node->keys[idx] = pred;
            return remove_internal(node->children[idx], pred);
        }
        // Else if right child has at least t keys, replace by successor
        else if (node->children[idx + 1]->n >= MIN_DEGREE) {
            Key succ = get_successor(node, idx);
            node->keys[idx] = succ;
            return remove_internal(node->children[idx + 1], succ);
        }
        // Else both children have t-1 keys => merge and recurse
        else {
            merge_children(node, idx);
            return remove_internal(node->children[idx], k);
        }
    }

    Key get_predecessor(Node* node, int idx) {
        Node* cur = node->children[idx];
        while (!cur->leaf) cur = cur->children[cur->n];
        return cur->keys[cur->n - 1];
    }

    Key get_successor(Node* node, int idx) {
        Node* cur = node->children[idx + 1];
        while (!cur->leaf) cur = cur->children[0];
        return cur->keys[0];
    }

    void fill(Node* node, int idx) {
        if (idx != 0 && node->children[idx - 1]->n >= MIN_DEGREE) {
            borrow_from_prev(node, idx);
        } else if (idx != node->n && node->children[idx + 1]->n >= MIN_DEGREE) {
            borrow_from_next(node, idx);
        } else {
            if (idx != node->n) merge_children(node, idx);
            else merge_children(node, idx - 1);
        }
    }

    int find_key_index(Node* node, const Key& key) const {
        int idx = 0;
        while (idx < node->n && comp(node->keys[idx], key)) ++idx;
        return idx;
    }
};

int main() {
    BTree234<int> tree;
    std::vector<int> vals = {20, 10, 5, 6, 12, 30, 7, 17};
    tree.batch_load(vals, true);

    std::cout << "Inorder after batch load: ";
    tree.inorder_traversal([](const int& x){ std::cout << x << ' '; });
    std::cout << "\nSize: " << tree.size() << "\n";

    tree.debug_print();

    std::cout << "Insert 3,4,2\n";
    tree.insert(3); tree.insert(4); tree.insert(2);
    tree.debug_print();

    std::cout << "Remove 6\n";
    tree.remove(6);
    tree.debug_print();

    std::cout << "Remove all\n";
    for (int i : {2,3,4,5,7,10,12,17,20,30}) tree.remove(i);
    std::cout << "After removing everything, size=" << tree.size() << "\n";
    tree.debug_print();

    return 0;
}
