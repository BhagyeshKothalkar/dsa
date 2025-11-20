#include <algorithm> 
#include <climits>  
#include <iostream>
#include <stdexcept>
#include <initializer_list>

template <typename T>
class ArrQueue {
private:
    T* m_arr;
    std::size_t m_max_size;
    std::size_t m_l; 
    std::size_t m_r;

public:
    ArrQueue(std::size_t capacity = 16);
    ArrQueue(std::initializer_list<T> data);
    ~ArrQueue();

    ArrQueue(const ArrQueue& other);
    ArrQueue& operator=(const ArrQueue& other);

    std::size_t size() const; 
    void enqueue(const T& val);
    void dequeue();

    T front() const; 
    T rear() const; 

    bool full() const;
    bool empty() const;
    void display() const;
};


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

template <typename T>
ArrQueue<T>::ArrQueue(const ArrQueue& other)
    : m_max_size(other.m_max_size), m_l(other.m_l), m_r(other.m_r) {
    m_arr = new T[m_max_size];
    for (std::size_t i = 0; i < m_max_size; ++i) {
        m_arr[i] = other.m_arr[i];
    }
}

template <typename T>
ArrQueue<T>& ArrQueue<T>::operator=(const ArrQueue& other) {
    if (this == &other) return *this;
    delete[] m_arr; 
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
    if (empty()) throw std::out_of_range("Queue is empty.");
    return m_arr[m_l];
}

template <typename T>
T ArrQueue<T>::rear() const {
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
  struct Node {
    T key;
    Node *left;
    Node *right;
    Node(T value) : key(value), left(nullptr), right(nullptr) {}
  };

  Node *m_root;

  Node *insert(Node *node, T key) {
    if (node == nullptr) {
      return new Node(key);
    }
    if (key < node->key) {
      node->left = insert(node->left, key);
    } else if (key > node->key) {
      node->right = insert(node->right, key);
    }
    return node;
  }

  Node *remove(Node *node, T key) {
    if (node == nullptr)
      return node;

    if (key < node->key) {
      node->left = remove(node->left, key);
    } else if (key > node->key) {
      node->right = remove(node->right, key);
    } else { 

      if (node->left == nullptr) {
        Node *temp = node->right;
        delete node;
        return temp;
      } else if (node->right == nullptr) {
        Node *temp = node->left;
        delete node;
        return temp;
      }

      Node *temp = findMinNode(node->right);
      node->key = temp->key;
      node->right = remove(node->right, temp->key);
    }
    return node;
  }


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


  int height(Node *node) const {
    if (node == nullptr) {
      return -1; 
    }
    return 1 + std::max(height(node->left), height(node->right));
  }

  
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


  bool checkBST(Node *node, T minVal, T maxVal) const {
    if (node == nullptr)
      return true;
    if (node->key <= minVal || node->key >= maxVal) {
      return false;
    }
    return checkBST(node->left, minVal, node->key) &&
           checkBST(node->right, node->key, maxVal);
  }


  Node *findMinNode(Node *node) const {
    while (node && node->left != nullptr) {
      node = node->left;
    }
    return node;
  }


  Node *findMaxNode(Node *node) const {
    while (node && node->right != nullptr) {
      node = node->right;
    }
    return node;
  }


  void destroyTree(Node *node) {
    if (node) {
      destroyTree(node->left);
      destroyTree(node->right);
      delete node;
    }
  }

public:

  BST() : m_root(nullptr) {}


  ~BST() { destroyTree(m_root); }


  Node *getRoot() const { return m_root; }



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


    while (curr != nullptr && curr->key != key) {
      if (key < curr->key) {
        successor = curr; 
        curr = curr->left;
      } else {
        curr = curr->right;
      }
    }

    if (curr == nullptr)
      return nullptr; 


    if (curr->right != nullptr) {
      return findMinNode(curr->right);
    }

    
    return successor;
  }
};
