#include <functional>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <cassert>
#include <optional>      // C++17
#include <utility>

template <typename T>
struct OrderStatisticAug {
    // property stored per node: subtree size
    using prop_type = std::size_t;

    template <typename Node>
    static void init(Node* n) {
        n->prop = static_cast<prop_type>(1);
    }

    template <typename Node>
    static void recompute(Node* n) {
        // n->left and n->right may be nullptr
        std::size_t left_sz  = n->left  ? static_cast<std::size_t>(n->left->prop)  : 0;
        std::size_t right_sz = n->right ? static_cast<std::size_t>(n->right->prop) : 0;
        n->prop = static_cast<prop_type>(1 + left_sz + right_sz);
    }
};

// ----------------- AVLTree with augmentation policy -----------------
template <
    typename T,
    typename Compare = std::less<T>,
    typename Aug = OrderStatisticAug<T>
>
class AVLTree {
private:
    struct Node {
        T value;
        Node* left;
        Node* right;
        int height;
        typename Aug::prop_type prop; // augmentation property

        explicit Node(const T& v)
            : value(v), left(nullptr), right(nullptr), height(1), prop() {}
    };

    Node* root;
    size_t node_count;
    Compare comp;

public:
    AVLTree() : root(nullptr), node_count(0), comp(Compare()) {}
    explicit AVLTree(Compare c) : root(nullptr), node_count(0), comp(c) {}
    ~AVLTree() { clear(); }

    // Public interface (as requested)
    bool insert(const T& value) {
        bool inserted = false;
        root = insert_rec(root, value, inserted);
        if (inserted) ++node_count;
        return inserted;
    }

    bool remove(const T& value) {
        bool removed = false;
        root = remove_rec(root, value, removed);
        if (removed) --node_count;
        return removed;
    }

    bool contains(const T& value) const {
        Node* cur = root;
        while (cur) {
            if (comp(value, cur->value)) cur = cur->left;
            else if (comp(cur->value, value)) cur = cur->right;
            else return true;
        }
        return false;
    }

    size_t size() const { return node_count; }

    void clear() {
        clear_rec(root);
        root = nullptr;
        node_count = 0;
    }

    void inorder_traversal(const std::function<void(const T&)>& fn) const {
        inorder_rec(root, fn);
    }

    // --------- Order-statistic helpers (provided because default augmentation is OS) ---------
    // Number of elements strictly less than value.
    size_t rank(const T& value) const {
        return rank_rec(root, value);
    }

    // Select k-th smallest (1-based). Returns std::nullopt if k==0 or k>size.
    std::optional<T> select(std::size_t k) const {
        if (k == 0 || k > node_count) return std::nullopt;
        Node* n = select_rec(root, k);
        if (!n) return std::nullopt;
        return n->value;
    }

private:
    // Utility accessors for augmentation property (interpreted as size by default).
    static std::size_t subtree_size(Node* n) {
        if (!n) return 0;
        // Convert prop to size_t (works if prop_type is size_t)
        return static_cast<std::size_t>(n->prop);
    }

    // Height helper
    static int height(Node* n) { return n ? n->height : 0; }
    static int balance_factor(Node* n) { return n ? height(n->left) - height(n->right) : 0; }

    // Update node: recompute height and augmentation property
    static void update_node(Node* n) {
        if (!n) return;
        n->height = 1 + std::max(height(n->left), height(n->right));
        Aug::template recompute<Node>(n);
    }

    // Rotations (member helpers) — update augmentation via update_node
    Node* rotateRight(Node* y) {
        assert(y && y->left);
        Node* x = y->left;
        Node* T2 = x->right;

        x->right = y;
        y->left  = T2;

        // Update affected nodes in bottom-up order
        update_node(y);
        update_node(x);
        return x;
    }

    Node* rotateLeft(Node* x) {
        assert(x && x->right);
        Node* y = x->right;
        Node* T2 = y->left;

        y->left  = x;
        x->right = T2;

        update_node(x);
        update_node(y);
        return y;
    }

    Node* rotateLeftRight(Node* n) {
        assert(n && n->left);
        n->left = rotateLeft(n->left);
        return rotateRight(n);
    }

    Node* rotateRightLeft(Node* n) {
        assert(n && n->right);
        n->right = rotateRight(n->right);
        return rotateLeft(n);
    }

    Node* rebalance(Node* n) {
        if (!n) return nullptr;
        update_node(n);
        int bf = balance_factor(n);

        if (bf > 1) {
            if (balance_factor(n->left) >= 0) {
                return rotateRight(n);         // LL
            } else {
                return rotateLeftRight(n);     // LR
            }
        }
        if (bf < -1) {
            if (balance_factor(n->right) <= 0) {
                return rotateLeft(n);          // RR
            } else {
                return rotateRightLeft(n);     // RL
            }
        }
        return n;
    }

    // Recursive insert/remove
    Node* insert_rec(Node* node, const T& value, bool& inserted) {
        if (!node) {
            Node* n = new Node(value);
            Aug::template init<Node>(n);
            inserted = true;
            return n;
        }
        if (comp(value, node->value)) {
            node->left = insert_rec(node->left, value, inserted);
        } else if (comp(node->value, value)) {
            node->right = insert_rec(node->right, value, inserted);
        } else {
            // duplicate: do not insert
            inserted = false;
            return node;
        }
        return rebalance(node);
    }

    static Node* find_min(Node* n) {
        assert(n);
        while (n->left) n = n->left;
        return n;
    }

    Node* remove_rec(Node* node, const T& value, bool& removed) {
        if (!node) {
            removed = false;
            return nullptr;
        }
        if (comp(value, node->value)) {
            node->left = remove_rec(node->left, value, removed);
        } else if (comp(node->value, value)) {
            node->right = remove_rec(node->right, value, removed);
        } else {
            // found
            removed = true;
            if (!node->left || !node->right) {
                Node* temp = node->left ? node->left : node->right;
                if (!temp) {
                    delete node;
                    return nullptr;
                } else {
                    Node* ret = temp;
                    delete node;
                    return ret;
                }
            } else {
                // two children: replace with inorder successor
                Node* succ = find_min(node->right);
                node->value = succ->value;
                node->right = remove_rec(node->right, succ->value, removed);
                // removed remains true
            }
        }
        return rebalance(node);
    }

    // k-th smallest select (1-based)
    Node* select_rec(Node* node, std::size_t k) const {
        if (!node) return nullptr;
        std::size_t left_sz = subtree_size(node->left);
        if (k == left_sz + 1) return node;
        if (k <= left_sz) return select_rec(node->left, k);
        return select_rec(node->right, k - left_sz - 1);
    }

    // rank: number of elements < value
    std::size_t rank_rec(Node* node, const T& value) const {
        if (!node) return 0;
        if (comp(value, node->value)) {
            return rank_rec(node->left, value);
        } else if (comp(node->value, value)) {
            std::size_t left_sz = subtree_size(node->left);
            return left_sz + 1 + rank_rec(node->right, value);
        } else {
            // equal: all in left subtree are strictly less
            return subtree_size(node->left);
        }
    }

    // Utilities
    static void clear_rec(Node* n) {
        if (!n) return;
        clear_rec(n->left);
        clear_rec(n->right);
        delete n;
    }

    static void inorder_rec(Node* n, const std::function<void(const T&)>& fn) {
        if (!n) return;
        inorder_rec(n->left, fn);
        fn(n->value);
        inorder_rec(n->right, fn);
    }

    // disable copies
    AVLTree(const AVLTree&) = delete;
    AVLTree& operator=(const AVLTree&) = delete;
};

int main() {
    AVLTree<int> avl;
    int vals[] = {30, 20, 40, 10, 25, 35, 50, 5, 15};
    for (int v : vals) avl.insert(v);

    std::cout << "In-order: ";
    avl.inorder_traversal([](const int& x){ std::cout << x << " "; });
    std::cout << "\nSize: " << avl.size() << "\n";

    std::cout << "Rank(25) = " << avl.rank(25) << " (expected 4)\n";
    auto maybe3 = avl.select(3);
    if (maybe3) std::cout << "3rd smallest = " << *maybe3 << "\n";

    avl.remove(20);
    std::cout << "After removing 20, rank(25) = " << avl.rank(25) << "\n";
    return 0;
}