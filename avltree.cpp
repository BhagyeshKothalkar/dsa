#include <functional>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <cassert>

template <typename T, typename Compare = std::less<T>>
class AVLTree {
private:
    struct Node {
        T value;
        Node* left;
        Node* right;
        int height;
        Node(const T& v) : value(v), left(nullptr), right(nullptr), height(1) {}
    };

    Node* root;
    size_t node_count;
    Compare comp;

public:
    AVLTree() : root(nullptr), node_count(0), comp(Compare()) {}
    explicit AVLTree(Compare c) : root(nullptr), node_count(0), comp(c) {}
    ~AVLTree() { clear(); }

    // Public interface
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

    // In-order traversal: apply fn to each value in ascending order
    void inorder_traversal(const std::function<void(const T&)>& fn) const {
        inorder_rec(root, fn);
    }

private:
    // Utility: node height, safe for null
    static int height(Node* n) { return n ? n->height : 0; }

    // Compute balance factor = height(left) - height(right)
    static int balance_factor(Node* n) { return n ? height(n->left) - height(n->right) : 0; }

    // Update height from children
    static void update_height(Node* n) {
        if (n) n->height = 1 + std::max(height(n->left), height(n->right));
    }

    // --- Rotations (member helpers) ---
    // Right rotation:
    //        y                x
    //       / \     ->       / \
    //      x   T3           T1  y
    //     / \                  / \
    //   T1  T2               T2  T3
    Node* rotateRight(Node* y) {
        assert(y != nullptr);
        Node* x = y->left;
        assert(x != nullptr);
        Node* T2 = x->right;

        // rotation
        x->right = y;
        y->left = T2;

        // update heights
        update_height(y);
        update_height(x);

        return x;
    }

    // Left rotation:
    //    x                 y
    //   / \    ->         / \
    //  T1  y             x  T3
    //     / \          / \
    //    T2 T3       T1  T2
    Node* rotateLeft(Node* x) {
        assert(x != nullptr);
        Node* y = x->right;
        assert(y != nullptr);
        Node* T2 = y->left;

        // rotation
        y->left = x;
        x->right = T2;

        // update heights
        update_height(x);
        update_height(y);

        return y;
    }

    // Left-Right rotation: first left on left child, then right on node
    Node* rotateLeftRight(Node* n) {
        assert(n && n->left);
        n->left = rotateLeft(n->left);
        return rotateRight(n);
    }

    // Right-Left rotation: first right on right child, then left on node
    Node* rotateRightLeft(Node* n) {
        assert(n && n->right);
        n->right = rotateRight(n->right);
        return rotateLeft(n);
    }

    // Rebalance a node after changes to its subtree
    Node* rebalance(Node* n) {
        if (!n) return nullptr;
        update_height(n);
        int bf = balance_factor(n);

        // Left heavy
        if (bf > 1) {
            if (balance_factor(n->left) >= 0) {
                // Left-Left case
                return rotateRight(n);
            } else {
                // Left-Right case
                return rotateLeftRight(n);
            }
        }
        // Right heavy
        if (bf < -1) {
            if (balance_factor(n->right) <= 0) {
                // Right-Right case
                return rotateLeft(n);
            } else {
                // Right-Left case
                return rotateRightLeft(n);
            }
        }
        // already balanced
        return n;
    }

    // Insert recursively. 'inserted' set to true if inserted, false if value already present.
    Node* insert_rec(Node* node, const T& value, bool& inserted) {
        if (!node) {
            inserted = true;
            return new Node(value);
        }

        if (comp(value, node->value)) {
            node->left = insert_rec(node->left, value, inserted);
        } else if (comp(node->value, value)) {
            node->right = insert_rec(node->right, value, inserted);
        } else {
            // duplicate -> do not insert
            inserted = false;
            return node;
        }

        return rebalance(node);
    }

    // Find node with minimum value in subtree
    static Node* find_min(Node* n) {
        assert(n != nullptr);
        while (n->left) n = n->left;
        return n;
    }

    // Remove recursively. 'removed' set to true if node deleted.
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
            // Found node to remove
            removed = true;

            // Node with only one child or no child
            if (!node->left || !node->right) {
                Node* temp = node->left ? node->left : node->right;
                if (!temp) {
                    // No child
                    delete node;
                    return nullptr;
                } else {
                    // One child: replace node with child
                    Node* toReturn = temp;
                    delete node;
                    return toReturn;
                }
            } else {
                // Node with two children: get inorder successor (min in right subtree)
                Node* succ = find_min(node->right);
                node->value = succ->value;
                // Delete the inorder successor in right subtree
                node->right = remove_rec(node->right, succ->value, removed);
                // Note: 'removed' already true; keep it true
            }
        }

        // One important nuance: when removing a node with two children, we set removed true
        // above; but the recursive call to remove the successor will also set removed true.
        // This is harmless. We avoid double-decrement by only decrementing in public remove
        // when removed was true before call. (We implemented remove() to decrement only once.)
        return rebalance(node);
    }

    // Recursively delete nodes
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

    // Disable copy to keep simple ownership semantics (could implement deep copy if desired)
    AVLTree(const AVLTree&) = delete;
    AVLTree& operator=(const AVLTree&) = delete;
};

// Example usage and simple test
int main() {
    AVLTree<int> avl;
    int vals[] = {30, 20, 40, 10, 25, 35, 50, 5, 15};
    for (int v : vals) {
        avl.insert(v);
    }

    std::cout << "In-order: ";
    avl.inorder_traversal([](const int& x){ std::cout << x << " "; });
    std::cout << "\nSize: " << avl.size() << "\n";

    std::cout << "Contains 25? " << (avl.contains(25) ? "yes" : "no") << "\n";
    std::cout << "Removing 20\n";
    avl.remove(20);

    std::cout << "In-order after deletion: ";
    avl.inorder_traversal([](const int& x){ std::cout << x << " "; });
    std::cout << "\nSize: " << avl.size() << "\n";

    // remove non-existing
    std::cout << "Remove 999 (not present): " << (avl.remove(999) ? "removed" : "not found") << "\n";

    return 0;
}
