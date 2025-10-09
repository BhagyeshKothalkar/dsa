#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>

// Forward declaration for the friend function
template <typename T> class CircDoublyLL;

// Friend function to allow `cout << list;`
template <typename T>
std::ostream &operator<<(std::ostream &os, const CircDoublyLL<T> &list);

template <typename T> class CircDoublyLL {
private:
  struct Node {
    T m_data;
    Node *m_next;
    Node *m_prev;
    Node(T data) : m_data(data), m_next(nullptr), m_prev(nullptr) {}
  };

  Node *m_head;
  size_t m_size;

  // Private helpers for Merge Sort
  Node *mergeSort(Node *head);
  Node *getMiddle(Node *head);
  Node *sortedMerge(Node *left, Node *right);

public:
  // --- Rule of Three/Five: Essential for memory management ---
  CircDoublyLL();
  ~CircDoublyLL();
  CircDoublyLL(const CircDoublyLL &other);
  CircDoublyLL<T> &operator=(const CircDoublyLL &other);

  // --- Accessors (all O(1)) ---
  T &front();
  const T &front() const;
  T &at(size_t pos);
  const T &at(size_t pos) const;
  T &back();
  const T &back() const;

  // --- Modifiers (all O(1)) ---
  void push_back(T data);
  void push_front(T data);
  void insert_at(T data, size_t pos);
  void pop_front();
  void pop_back();
  void pop_at(size_t pos);
  void pop_val(T val);
  void reverse();

  // --- Utility ---
  size_t find(T val) const;
  size_t size() const;
  bool isempty() const;
  void sort();

  friend std::ostream &operator<< <>(std::ostream &os,
                                      const CircDoublyLL<T> &list);
};

// =================================================================
// IMPLEMENTATIONS
// =================================================================

// --- Rule of Three/Five ---

template <typename T>
CircDoublyLL<T>::CircDoublyLL() : m_head(nullptr), m_size(0) {}

template <typename T> CircDoublyLL<T>::~CircDoublyLL() {
  while (!isempty()) {
    pop_front();
  }
}

template <typename T>
CircDoublyLL<T>::CircDoublyLL(const CircDoublyLL &other)
    : m_head(nullptr), m_size(0) {
  if (other.isempty()) return;
  Node* current = other.m_head;
  for(size_t i = 0; i < other.m_size; ++i) {
    push_back(current->m_data);
    current = current->m_next;
  }
}

template <typename T>
CircDoublyLL<T> &CircDoublyLL<T>::operator=(const CircDoublyLL &other) {
  if (this == &other) return *this;
  while (!isempty()) pop_front();

  if (other.isempty()) return *this;
  Node* current = other.m_head;
  for(size_t i = 0; i < other.m_size; ++i) {
    push_back(current->m_data);
    current = current->m_next;
  }
  return *this;
}

// --- Accessors ---

template <typename T> T &CircDoublyLL<T>::front() {
  if (isempty()) throw std::out_of_range("front() on empty list");
  return m_head->m_data;
}
template <typename T> const T &CircDoublyLL<T>::front() const {
  if (isempty()) throw std::out_of_range("front() on empty list");
  return m_head->m_data;
}

template <typename T> T &CircDoublyLL<T>::back() {
  if (isempty()) throw std::out_of_range("back() on empty list");
  return m_head->m_prev->m_data;
}
template <typename T> const T &CircDoublyLL<T>::back() const {
  if (isempty()) throw std::out_of_range("back() on empty list");
  return m_head->m_prev->m_data;
}

template <typename T> T &CircDoublyLL<T>::at(size_t pos) {
  if (pos >= m_size) throw std::out_of_range("Index out of range");
  Node *current;
  if (pos < m_size / 2) {
    current = m_head;
    for (size_t i = 0; i < pos; ++i) current = current->m_next;
  } else {
    current = m_head->m_prev; // Start at tail
    for (size_t i = 0; i < m_size - 1 - pos; ++i) current = current->m_prev;
  }
  return current->m_data;
}
template <typename T> const T &CircDoublyLL<T>::at(size_t pos) const {
  return const_cast<CircDoublyLL<T>*>(this)->at(pos);
}

// --- Modifiers ---

template <typename T> void CircDoublyLL<T>::push_front(T data) {
  Node *newNode = new Node(data);
  if (isempty()) {
    newNode->m_next = newNode;
    newNode->m_prev = newNode;
    m_head = newNode;
  } else {
    Node *tail = m_head->m_prev;
    newNode->m_next = m_head;
    newNode->m_prev = tail;
    m_head->m_prev = newNode;
    tail->m_next = newNode;
    m_head = newNode; // The new node is the new head
  }
  m_size++;
}

template <typename T> void CircDoublyLL<T>::push_back(T data) {
  // Pushing back is the same as pushing front, but without updating the head
  push_front(data);
  if (m_size > 1) {
    m_head = m_head->m_next;
  }
}

template <typename T> void CircDoublyLL<T>::insert_at(T data, size_t pos) {
  if (pos > m_size) throw std::out_of_range("Cannot insert out of range");
  if (pos == 0) { push_front(data); return; }
  if (pos == m_size) { push_back(data); return; }

  Node *current = m_head;
  for (size_t i = 0; i < pos; ++i) current = current->m_next;

  Node *prev_node = current->m_prev;
  Node *newNode = new Node(data);
  newNode->m_next = current;
  newNode->m_prev = prev_node;
  prev_node->m_next = newNode;
  current->m_prev = newNode;
  m_size++;
}

template <typename T> void CircDoublyLL<T>::pop_front() {
  if (isempty()) throw std::out_of_range("pop_front() on empty list");
  if (m_size == 1) {
    delete m_head;
    m_head = nullptr;
  } else {
    Node *tail = m_head->m_prev;
    Node *newHead = m_head->m_next;
    tail->m_next = newHead;
    newHead->m_prev = tail;
    delete m_head;
    m_head = newHead;
  }
  m_size--;
}

template <typename T> void CircDoublyLL<T>::pop_back() {
  if (isempty()) throw std::out_of_range("pop_back() on empty list");
  if (m_size == 1) { pop_front(); return; }

  Node *oldTail = m_head->m_prev;
  Node *newTail = oldTail->m_prev;
  newTail->m_next = m_head;
  m_head->m_prev = newTail;
  delete oldTail;
  m_size--;
}

template <typename T> void CircDoublyLL<T>::pop_at(size_t pos) {
  if (pos >= m_size) throw std::out_of_range("Cannot pop out of range");
  if (pos == 0) { pop_front(); return; }
  if (pos == m_size - 1) { pop_back(); return; }

  Node* toDelete = m_head;
  for (size_t i = 0; i < pos; ++i) toDelete = toDelete->m_next;

  toDelete->m_prev->m_next = toDelete->m_next;
  toDelete->m_next->m_prev = toDelete->m_prev;
  delete toDelete;
  m_size--;
}

template <typename T> void CircDoublyLL<T>::pop_val(T val) {
  size_t pos = find(val);
  if (pos < m_size) pop_at(pos);
}

template <typename T> void CircDoublyLL<T>::reverse() {
  if (m_size < 2) return;
  Node* current = m_head;
  for(size_t i = 0; i < m_size; ++i) {
      Node* temp = current->m_next;
      current->m_next = current->m_prev;
      current->m_prev = temp;
      current = temp; // Move to the original next
  }
  m_head = m_head->m_next; // The old tail is the new head
}


// --- Utility ---

template <typename T> size_t CircDoublyLL<T>::find(T val) const {
  if (isempty()) return 0;
  Node *current = m_head;
  for(size_t i = 0; i < m_size; ++i) {
      if(current->m_data == val) return i;
      current = current->m_next;
  }
  return m_size;
}

template <typename T> size_t CircDoublyLL<T>::size() const { return m_size; }
template <typename T> bool CircDoublyLL<T>::isempty() const { return m_head == nullptr; }

template <typename T>
std::ostream &operator<<(std::ostream &os, const CircDoublyLL<T> &list) {
  if (list.isempty()) { os << "[]"; return os; }
  os << "[";
  typename CircDoublyLL<T>::Node *current = list.m_head;
  for (size_t i = 0; i < list.m_size; ++i) {
    os << current->m_data;
    if (i < list.m_size - 1) os << " <-> ";
    current = current->m_next;
  }
  os << "]";
  return os;
}

// --- Merge Sort ---

template <typename T> void CircDoublyLL<T>::sort() {
    if (m_size < 2) return;

    // 1. Break the circle to form a linear doubly-linked list
    Node* tail = m_head->m_prev;
    tail->m_next = nullptr;
    m_head->m_prev = nullptr;

    // 2. Sort the linear list (this only considers m_next pointers)
    m_head = mergeSort(m_head);

    // 3. Fix all the m_prev pointers and find the new tail
    Node* current = m_head;
    Node* prev = nullptr;
    while (current != nullptr) {
        current->m_prev = prev;
        prev = current;
        current = current->m_next;
    }
    Node* newTail = prev; // The last non-null node is the new tail

    // 4. Re-link the circle
    newTail->m_next = m_head;
    m_head->m_prev = newTail;
}

// The following helpers are identical to the linear DoublyLinkedList version
template <typename T>
typename CircDoublyLL<T>::Node *CircDoublyLL<T>::mergeSort(Node *head) {
  if (!head || !head->m_next) return head;
  Node *middle = getMiddle(head);
  Node *right_half_head = middle->m_next;
  middle->m_next = nullptr;
  Node *sorted_left = mergeSort(head);
  Node *sorted_right = mergeSort(right_half_head);
  return sortedMerge(sorted_left, sorted_right);
}

template <typename T>
typename CircDoublyLL<T>::Node *CircDoublyLL<T>::getMiddle(Node *head) {
  if (!head) return head;
  Node *slow = head, *fast = head;
  while (fast->m_next != nullptr && fast->m_next->m_next != nullptr) {
    slow = slow->m_next;
    fast = fast->m_next->m_next;
  }
  return slow;
}

template <typename T>
typename CircDoublyLL<T>::Node *CircDoublyLL<T>::sortedMerge(Node *left, Node *right) {
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
  CircDoublyLL<int> list;

  // Test 1: push_back and push_front (all O(1))
  for (int i = 0; i < 5000; ++i) {
    list.push_back(i);
    list.push_front(i * -1);
  }
  assert(list.size() == 10000);
  assert(list.front() == -4999);
  assert(list.back() == 4999);
  std::cout << "Test 1 Passed: push_back and push_front OK." << std::endl;

  // Test 2: pop_back and pop_front (all O(1))
  for (int i = 0; i < 2000; ++i) {
    list.pop_back();
    list.pop_front();
  }
  assert(list.size() == 6000);
  assert(list.front() == -2999);
  assert(list.back() == 2999);
  std::cout << "Test 2 Passed: pop_back and pop_front OK." << std::endl;

  // Test 3: insert_at and at()
  list.insert_at(9999, list.size()); // end
  list.insert_at(-9999, 0);          // front
  list.insert_at(0, 3000);           // middle
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
  for(size_t i = 0; i < 10; ++i) {
      assert(list.at(i) <= list.at(i+1));
  }
  std::cout << "Test 5 Passed: sort() OK." << std::endl;

  std::cout << "\nFinal list state (first 10 and last 10 elements):\n";
  for (size_t i = 0; i < 10; ++i) std::cout << list.at(i) << " ";
  std::cout << "... ";
  for (size_t i = list.size() - 10; i < list.size(); ++i) std::cout << list.at(i) << " ";
  std::cout << std::endl;
  
  std::cout << "\nAll stress tests completed successfully." << std::endl;

  return 0;
}