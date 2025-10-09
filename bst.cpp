#include <algorithm> // For std::max
#include <climits>   // For INT_MIN, INT_MAX
#include <iostream>
#include <stdexcept> // For exceptions
#include <initializer_list>
#include <string>

template <typename T>
class ArrQueue {
private:
    T* m_arr;
    std::size_t m_max_size;
    std::size_t m_l; // Index of the first element
    std::size_t m_r; // Index for the next insertion

public:
    // FIX: Default constructor now creates a queue with a default capacity.
    ArrQueue(std::size_t capacity = 16);
    ArrQueue(std::initializer_list<T> data);
    ~ArrQueue();

    // FIX: Added copy constructor and assignment operator to prevent shallow copies.
    ArrQueue(const ArrQueue& other);
    ArrQueue& operator=(const ArrQueue& other);

    std::size_t size() const; // Made const-correct
    void enqueue(const T& val);
    void dequeue();

    T front() const; // Made const-correct
    T rear() const;  // Made const-correct

    bool full() const;   // Made const-correct
    bool empty() const;  // Made const-correct
    void display() const; // Made const-correct
};

// --- Implementations ---

template <typename T>
ArrQueue<T>::ArrQueue(std::size_t capacity)
    : m_max_size(capacity + 1), m_l(0), m_r(0) {
    if (capacity == 0) throw std::invalid_argument("Capacity cannot be zero.");
    m_arr = new T[m_max_size];
}

template <typename T>
ArrQueue<T>::ArrQueue(std::initializer_list<T> data)
    : m_max_size(data.size() + 1), m_l(0), m_r(data.size()) {
    m_arr = new T[m_max_size];
    std::size_t i = 0;
    for (const T& item : data) {
        m_arr[i++] = item;
    }
}

template <typename T>
ArrQueue<T>::~ArrQueue() {
    delete[] m_arr;
}

// Copy constructor for deep copy
template <typename T>
ArrQueue<T>::ArrQueue(const ArrQueue& other)
    : m_max_size(other.m_max_size), m_l(other.m_l), m_r(other.m_r) {
    m_arr = new T[m_max_size];
    for (std::size_t i = 0; i < m_max_size; ++i) {
        m_arr[i] = other.m_arr[i];
    }
}

// Copy assignment operator for deep copy
template <typename T>
ArrQueue<T>& ArrQueue<T>::operator=(const ArrQueue& other) {
    if (this == &other) return *this;
    delete[] m_arr; // Free old memory
    m_max_size = other.m_max_size;
    m_l = other.m_l;
    m_r = other.m_r;
    m_arr = new T[m_max_size];
    for (std::size_t i = 0; i < m_max_size; ++i) {
        m_arr[i] = other.m_arr[i];
    }
    return *this;
}


template <typename T>
std::size_t ArrQueue<T>::size() const {
    return (m_max_size - m_l + m_r) % m_max_size;
}

template <typename T>
void ArrQueue<T>::enqueue(const T& val) {
    if (full()) throw std::out_of_range("Queue is full.");
    m_arr[m_r] = val;
    m_r = (m_r + 1) % m_max_size;
}

template <typename T>
void ArrQueue<T>::dequeue() {
    if (empty()) throw std::out_of_range("Queue is empty.");
    m_l = (m_l + 1) % m_max_size;
}

template <typename T>
bool ArrQueue<T>::full() const {
    return (m_r + 1) % m_max_size == m_l;
}

template <typename T>
bool ArrQueue<T>::empty() const {
    return m_l == m_r;
}

template <typename T>
T ArrQueue<T>::front() const {
    // FIX: Check for empty queue before access.
    if (empty()) throw std::out_of_range("Queue is empty.");
    return m_arr[m_l];
}

template <typename T>
T ArrQueue<T>::rear() const {
    // FIX: Check for empty queue and use safe modulo arithmetic.
    if (empty()) throw std::out_of_range("Queue is empty.");
    return m_arr[(m_r - 1 + m_max_size) % m_max_size];
}

template <typename T>
void ArrQueue<T>::display() const {
    if (empty()) {
        std::cout << "[empty]" << std::endl;
        return;
    }
    std::size_t t = m_l;
    while (t != m_r) {
        std::cout << m_arr[t] << ' ';
        t = (t + 1) % m_max_size;
    }
    std::cout << std::endl;
}


template <typename T> class BST {
private:
  // The basic building block of the tree
  struct Node {
    T key;
    Node *left;
    Node *right;
    Node(T value) : key(value), left(nullptr), right(nullptr) {}
  };

  Node *m_root;

  // Helper for inserting a key
  Node *insert(Node *node, T key) {
    if (node == nullptr) {
      return new Node(key);
    }
    if (key < node->key) {
      node->left = insert(node->left, key);
    } else if (key > node->key) {
      node->right = insert(node->right, key);
    }
    return node; // Return the unchanged node pointer
  }

  // Helper for removing a key
  Node *remove(Node *node, T key) {
    if (node == nullptr)
      return node;

    if (key < node->key) {
      node->left = remove(node->left, key);
    } else if (key > node->key) {
      node->right = remove(node->right, key);
    } else { // Key found
      // Case 1 & 2: Node with one or no child
      if (node->left == nullptr) {
        Node *temp = node->right;
        delete node;
        return temp;
      } else if (node->right == nullptr) {
        Node *temp = node->left;
        delete node;
        return temp;
      }
      // Case 3: Node with two children
      Node *temp = findMinNode(node->right);
      node->key = temp->key;
      node->right = remove(node->right, temp->key);
    }
    return node;
  }

  // Helper for searching for a key
  bool search(Node *node, T key) const {
    if (node == nullptr) {
      return false;
    }
    if (node->key == key) {
      return true;
    }
    if (key < node->key) {
      return search(node->left, key);
    } else {
      return search(node->right, key);
    }
  }

  // Traversal helpers
  void preorder(Node *node) const {
    if (node == nullptr)
      return;
    std::cout << node->key << " ";
    preorder(node->left);
    preorder(node->right);
  }

  void inorder(Node *node) const {
    if (node == nullptr)
      return;
    inorder(node->left);
    std::cout << node->key << " ";
    inorder(node->right);
  }

  void postorder(Node *node) const {
    if (node == nullptr)
      return;
    postorder(node->left);
    postorder(node->right);
    std::cout << node->key << " ";
  }

  // Helper to calculate height of a subtree
  int height(Node *node) const {
    if (node == nullptr) {
      return -1; // Height of an empty tree is -1
    }
    return 1 + std::max(height(node->left), height(node->right));
  }

  // Helper to find depth of a given node
  int depth(Node *current, Node *target_node, int d) const {
    if (current == nullptr)
      return -1; // Not found
    if (current == target_node)
      return d;

    int left_depth = depth(current->left, target_node, d + 1);
    if (left_depth != -1)
      return left_depth;

    return depth(current->right, target_node, d + 1);
  }

  // Helper to validate the BST property
  bool checkBST(Node *node, T minVal, T maxVal) const {
    if (node == nullptr)
      return true;
    if (node->key <= minVal || node->key >= maxVal) {
      return false;
    }
    return checkBST(node->left, minVal, node->key) &&
           checkBST(node->right, node->key, maxVal);
  }

  // Helper to find the node with the minimum key in a subtree
  Node *findMinNode(Node *node) const {
    while (node && node->left != nullptr) {
      node = node->left;
    }
    return node;
  }

  // Helper to find the node with the maximum key in a subtree
  Node *findMaxNode(Node *node) const {
    while (node && node->right != nullptr) {
      node = node->right;
    }
    return node;
  }

  // Helper to clean up memory
  void destroyTree(Node *node) {
    if (node) {
      destroyTree(node->left);
      destroyTree(node->right);
      delete node;
    }
  }

public:
  // Constructor
  BST() : m_root(nullptr) {}

  // Destructor
  ~BST() { destroyTree(m_root); }

  // Public getter for the root (useful for some algorithms)
  Node *getRoot() const { return m_root; }

  // =========== PUBLIC INTERFACE ===========

  void insert(T key) { m_root = insert(m_root, key); }
  void remove(T key) { m_root = remove(m_root, key); }
  bool search(T key) const { return search(m_root, key); }

  void preorder() const {
    preorder(m_root);
    std::cout << std::endl;
  }
  void inorder() const {
    inorder(m_root);
    std::cout << std::endl;
  }
  void postorder() const {
    postorder(m_root);
    std::cout << std::endl;
  }

  void levelorder() const {
    if (m_root == nullptr) {
      std::cout << "[empty]" << std::endl;
      return;
    }

    // Use your custom ArrQueue to hold Node pointers
    ArrQueue<Node *> q(100);

    q.enqueue(m_root);

    while (!q.empty()) {
      Node *curr = q.front();
      q.dequeue();

      std::cout << curr->key << " ";

      if (curr->left != nullptr) {
        q.enqueue(curr->left);
      }
      if (curr->right != nullptr) {
        q.enqueue(curr->right);
      }
    }
    std::cout << std::endl;
  }

  int height() const { return height(m_root); }

  // Depth of a specific node pointer
  int depth(Node *target_node) const { return depth(m_root, target_node, 0); }

  T minimum() const {
    if (m_root == nullptr)
      throw std::runtime_error("Tree is empty");
    return findMinNode(m_root)->key;
  }

  T maximum() const {
    if (m_root == nullptr)
      throw std::runtime_error("Tree is empty");
    return findMaxNode(m_root)->key;
  }

  bool isBST() const {
    return checkBST(m_root, std::numeric_limits<T>::min(),
                    std::numeric_limits<T>::max());
  }

  Node *inorderSuccessor(T key) const {
    Node *curr = m_root;
    Node *successor = nullptr;

    // 1. Search for the node with the given key
    while (curr != nullptr && curr->key != key) {
      if (key < curr->key) {
        successor = curr; // This is a potential successor
        curr = curr->left;
      } else {
        curr = curr->right;
      }
    }

    if (curr == nullptr)
      return nullptr; // Key not found

    // 2. If a right subtree exists, find its minimum
    if (curr->right != nullptr) {
      return findMinNode(curr->right);
    }

    // 3. Otherwise, the successor is the last ancestor we turned left from
    return successor;
  }
};