#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>

// Forward declaration for the friend function
template <typename T> class CircSinglyLL;

// Friend function to allow `cout << list;`
template <typename T>
std::ostream &operator<<(std::ostream &os, const CircSinglyLL<T> &list);

template <typename T> class CircSinglyLL {
private:
  struct Node {
    T m_data;
    Node *m_next;
    Node(T data) : m_data(data), m_next(nullptr) {}
  };

  // Using a tail pointer is more efficient for circular lists.
  // The head is always accessible via `m_tail->m_next`.
  Node *m_tail;
  size_t m_size;

  // Private helpers for Merge Sort
  Node *mergeSort(Node *head);
  Node *getMiddle(Node *head);
  Node *sortedMerge(Node *left, Node *right);

public:
  // --- Rule of Three/Five: Essential for memory management ---
  CircSinglyLL();
  ~CircSinglyLL();
  CircSinglyLL(const CircSinglyLL &other);
  CircSinglyLL<T> &operator=(const CircSinglyLL &other);

  // --- Accessors (front() and back() are O(1)) ---
  T &front();
  const T &front() const;
  T &at(size_t pos);
  const T &at(size_t pos) const;
  T &back();
  const T &back() const;

  // --- Modifiers (push_front() and push_back() are O(1)) ---
  void push_back(T data);
  void push_front(T data);
  void insert_at(T data, size_t pos);
  void pop_front();
  void pop_back(); // O(n) operation
  void pop_at(size_t pos);
  void pop_val(T val);
  void reverse();

  // --- Utility ---
  size_t find(T val) const;
  size_t size() const;
  bool isempty() const;
  void sort();

  friend std::ostream &operator<< <>(std::ostream &os,
                                      const CircSinglyLL<T> &list);
};

// =================================================================
// IMPLEMENTATIONS
// =================================================================

// --- Rule of Three/Five ---

template <typename T>
CircSinglyLL<T>::CircSinglyLL() : m_tail(nullptr), m_size(0) {}

template <typename T> CircSinglyLL<T>::~CircSinglyLL() {
  while (!isempty()) {
    pop_front();
  }
}

template <typename T>
CircSinglyLL<T>::CircSinglyLL(const CircSinglyLL &other)
    : m_tail(nullptr), m_size(0) {
    if (other.isempty()) return;
    Node* current = other.m_tail->m_next; // Start at other's head
    for(size_t i = 0; i < other.m_size; ++i) {
        push_back(current->m_data);
        current = current->m_next;
    }
}

template <typename T>
CircSinglyLL<T> &CircSinglyLL<T>::operator=(const CircSinglyLL &other) {
  if (this == &other) return *this;
  while (!isempty()) pop_front();

  if (other.isempty()) return *this;
  Node* current = other.m_tail->m_next;
  for(size_t i = 0; i < other.m_size; ++i) {
      push_back(current->m_data);
      current = current->m_next;
  }
  return *this;
}

// --- Accessors ---

template <typename T> T &CircSinglyLL<T>::front() {
  if (isempty()) throw std::out_of_range("front() on empty list");
  return m_tail->m_next->m_data;
}
template <typename T> const T &CircSinglyLL<T>::front() const {
  if (isempty()) throw std::out_of_range("front() on empty list");
  return m_tail->m_next->m_data;
}

template <typename T> T &CircSinglyLL<T>::back() {
  if (isempty()) throw std::out_of_range("back() on empty list");
  return m_tail->m_data;
}
template <typename T> const T &CircSinglyLL<T>::back() const {
  if (isempty()) throw std::out_of_range("back() on empty list");
  return m_tail->m_data;
}

template <typename T> T &CircSinglyLL<T>::at(size_t pos) {
  if (pos >= m_size) throw std::out_of_range("Index out of range");
  Node *current = m_tail->m_next; // Start at head
  for (size_t i = 0; i < pos; ++i) {
    current = current->m_next;
  }
  return current->m_data;
}
template <typename T> const T &CircSinglyLL<T>::at(size_t pos) const {
    if (pos >= m_size) throw std::out_of_range("Index out of range");
    return const_cast<CircSinglyLL<T>*>(this)->at(pos);
}

// --- Modifiers ---

template <typename T> void CircSinglyLL<T>::push_front(T data) {
  Node *newNode = new Node(data);
  if (isempty()) {
    m_tail = newNode;
    m_tail->m_next = m_tail; // Point to itself
  } else {
    newNode->m_next = m_tail->m_next; // New node points to old head
    m_tail->m_next = newNode;         // Tail points to new head
  }
  m_size++;
}

template <typename T> void CircSinglyLL<T>::push_back(T data) {
  push_front(data);
  if (m_size > 1) {
    m_tail = m_tail->m_next; // The new node becomes the new tail
  }
}

template <typename T> void CircSinglyLL<T>::insert_at(T data, size_t pos) {
  if (pos > m_size) throw std::out_of_range("Cannot insert out of range");
  if (pos == 0) { push_front(data); return; }
  if (pos == m_size) { push_back(data); return; }

  Node* prev = m_tail->m_next; // Start at head
  for (size_t i = 0; i < pos - 1; ++i) {
    prev = prev->m_next;
  }
  Node* newNode = new Node(data);
  newNode->m_next = prev->m_next;
  prev->m_next = newNode;
  m_size++;
}

template <typename T> void CircSinglyLL<T>::pop_front() {
  if (isempty()) throw std::out_of_range("pop_front() on empty list");
  if (m_size == 1) {
    delete m_tail;
    m_tail = nullptr;
  } else {
    Node *oldHead = m_tail->m_next;
    m_tail->m_next = oldHead->m_next; // Tail points to the new head
    delete oldHead;
  }
  m_size--;
}

template <typename T> void CircSinglyLL<T>::pop_back() {
  if (isempty()) throw std::out_of_range("pop_back() on empty list");
  if (m_size == 1) {
      pop_front();
      return;
  }

  // O(n) traversal to find the node before the tail
  Node* prev = m_tail->m_next;
  while(prev->m_next != m_tail) {
      prev = prev->m_next;
  }
  prev->m_next = m_tail->m_next; // New tail points to head
  delete m_tail;
  m_tail = prev; // Update tail pointer
  m_size--;
}

template <typename T> void CircSinglyLL<T>::pop_at(size_t pos) {
  if (pos >= m_size) throw std::out_of_range("Cannot pop out of range");
  if (pos == 0) { pop_front(); return; }
  if (pos == m_size - 1) { pop_back(); return; }

  Node* prev = m_tail->m_next;
  for (size_t i = 0; i < pos - 1; ++i) {
    prev = prev->m_next;
  }
  Node* toDelete = prev->m_next;
  prev->m_next = toDelete->m_next;
  delete toDelete;
  m_size--;
}

template <typename T> void CircSinglyLL<T>::pop_val(T val) {
  size_t pos = find(val);
  if (pos < m_size) {
    pop_at(pos);
  }
}

template <typename T> void CircSinglyLL<T>::reverse() {
  if (m_size < 2) return;

  Node* prev = m_tail;
  Node* current = m_tail->m_next; // head
  Node* next_node = nullptr;

  for(size_t i = 0; i < m_size; ++i) {
      next_node = current->m_next;
      current->m_next = prev;
      prev = current;
      current = next_node;
  }
  // The original head is the new tail
  m_tail = current;
}


// --- Utility ---

template <typename T> size_t CircSinglyLL<T>::find(T val) const {
  if (isempty()) return 0;
  Node *current = m_tail->m_next;
  for(size_t i = 0; i < m_size; ++i) {
      if(current->m_data == val) return i;
      current = current->m_next;
  }
  return m_size; // Not found
}

template <typename T> size_t CircSinglyLL<T>::size() const { return m_size; }
template <typename T> bool CircSinglyLL<T>::isempty() const { return m_tail == nullptr; }

template <typename T>
std::ostream &operator<<(std::ostream &os, const CircSinglyLL<T> &list) {
  if (list.isempty()) {
    os << "[]";
    return os;
  }
  os << "[";
  typename CircSinglyLL<T>::Node *current = list.m_tail->m_next; // Start at head
  for (size_t i = 0; i < list.m_size; ++i) {
    os << current->m_data;
    if (i < list.m_size - 1) os << " -> ";
    current = current->m_next;
  }
  os << "]";
  return os;
}

// --- Merge Sort ---
// Implemented by temporarily breaking the circle, sorting, and re-linking

template <typename T> void CircSinglyLL<T>::sort() {
  if (m_size < 2) return;

  // 1. Break the circle to form a linear list
  Node* head = m_tail->m_next;
  m_tail->m_next = nullptr;

  // 2. Sort the linear list using the standard merge sort
  head = mergeSort(head);

  // 3. Find the new tail and re-link the circle
  m_tail = head;
  while(m_tail->m_next != nullptr) {
      m_tail = m_tail->m_next;
  }
  m_tail->m_next = head;
}

// The following helpers are identical to the SinglyLinkedList version
template <typename T>
typename CircSinglyLL<T>::Node *CircSinglyLL<T>::mergeSort(Node *head) {
  if (!head || !head->m_next) return head;
  Node *middle = getMiddle(head);
  Node *right_half_head = middle->m_next;
  middle->m_next = nullptr;
  Node *sorted_left = mergeSort(head);
  Node *sorted_right = mergeSort(right_half_head);
  return sortedMerge(sorted_left, sorted_right);
}

template <typename T>
typename CircSinglyLL<T>::Node *CircSinglyLL<T>::getMiddle(Node *head) {
  if (!head) return head;
  Node *slow = head, *fast = head;
  while (fast->m_next != nullptr && fast->m_next->m_next != nullptr) {
    slow = slow->m_next;
    fast = fast->m_next->m_next;
  }
  return slow;
}

template <typename T>
typename CircSinglyLL<T>::Node *
CircSinglyLL<T>::sortedMerge(Node *left, Node *right) {
  if (!left) return right;
  if (!right) return left;
  Node *result = nullptr;
  if (left->m_data <= right->m_data) {
    result = left;
    result->m_next = sortedMerge(left->m_next, right);
  } else {
    result = right;
    result->m_next = sortedMerge(left, right->m_next);
  }
  return result;
}

// --- Stress Test Main ---

int main() {
  CircSinglyLL<int> list;

  // Test 1: push_back and push_front (O(1) operations)
  for (int i = 0; i < 5000; ++i) {
    list.push_back(i);
    list.push_front(i * -1);
  }
  assert(list.size() == 10000);
  assert(list.front() == -4999);
  assert(list.back() == 4999);
  std::cout << "Test 1 Passed: push_back and push_front OK." << std::endl;

  // Test 2: pop_front (O(1)) and pop_back (O(n))
  for (int i = 0; i < 2000; ++i) {
    list.pop_back();
    list.pop_front();
  }
  assert(list.size() == 6000);
  assert(list.front() == -2999);
  assert(list.back() == 2999);
  std::cout << "Test 2 Passed: pop_back and pop_front OK." << std::endl;

  // Test 3: insert_at and at()
  list.insert_at(9999, list.size()); // insert at end
  list.insert_at(-9999, 0);          // insert at front
  list.insert_at(0, 3000);           // insert in middle
  assert(list.size() == 6003);
  assert(list.front() == -9999);
  assert(list.at(3000) == 0);
  assert(list.back() == 9999);
  std::cout << "Test 3 Passed: insert_at and at() OK." << std::endl;
  
  // Test 4: Reverse
  int old_front = list.front();
  int old_back = list.back();
  list.reverse();
  assert(list.front() == old_back);
  assert(list.back() == old_front);
  list.reverse(); // reverse back
  assert(list.front() == old_front);
  assert(list.back() == old_back);
  std::cout << "Test 4 Passed: reverse() OK." << std::endl;

  // Test 5: Sort
  list.sort();
  assert(list.front() == -9999);
  assert(list.back() == 9999);
  // Check if first few elements are sorted
  for(size_t i = 0; i < 10; ++i) {
      assert(list.at(i) <= list.at(i+1));
  }
  std::cout << "Test 5 Passed: sort() OK." << std::endl;
  
  std::cout << "\nFinal list state (first 20 elements): ";
  for (size_t i = 0; i < 20; ++i) {
    std::cout << list.at(i) << " ";
  }
  std::cout << "..." << std::endl;
  std::cout << "Final list looks like: " << list << std::endl;
  
  std::cout << "\nAll stress tests completed successfully." << std::endl;

  return 0;
}