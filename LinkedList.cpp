#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>

// Forward declaration for the friend function
template <typename T> class LinkedList;

// Friend function to allow `cout << list;`
template <typename T>
std::ostream &operator<<(std::ostream &os, const LinkedList<T> &list);

template <typename T> class LinkedList {
private:
  struct Node {
    T m_data;
    Node *m_next;
    Node(T data) : m_data(data), m_next(nullptr) {}
  };
  Node *m_head;
  size_t m_size;

  Node *mergeSort(Node *head);
  Node *getMiddle(Node *head);
  Node *sortedMerge(Node *left, Node *right);

public:
  LinkedList();
  ~LinkedList();
  LinkedList(const LinkedList &other);
  LinkedList<T> &operator=(const LinkedList &other);

  // --- Accessors ---
  T &front();
  const T &front() const;
  T &at(size_t pos);
  const T &at(size_t pos) const;
  T &back();
  const T &back() const;

  // --- Modifiers ---
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
  void printlist() const;
  size_t size() const;
  bool isempty() const;
  void sort();
  friend std::ostream &operator<< <>(std::ostream &os,
                                     const LinkedList<T> &list);
};

template <typename T>
LinkedList<T>::LinkedList() : m_head(nullptr), m_size(0) {}

template <typename T> LinkedList<T>::~LinkedList() {
  while (!isempty()) {
    pop_front();
  }
}

template <typename T>
LinkedList<T>::LinkedList(const LinkedList &other)
    : m_head(nullptr), m_size(0) {
  // Perform a deep copy by iterating through the other list
  for (Node *current = other.m_head; current != nullptr;
       current = current->m_next) {
    push_back(current->m_data);
  }
}

template <typename T>
LinkedList<T> &LinkedList<T>::operator=(const LinkedList &other) {
  if (this == &other) { // Handle self-assignment
    return *this;
  }
  // Clear current list and then deep copy
  while (!isempty()) {
    pop_front();
  }
  for (Node *current = other.m_head; current != nullptr;
       current = current->m_next) {
    push_back(current->m_data);
  }
  return *this;
}

// --- Accessors ---

template <typename T> T &LinkedList<T>::front() {
  if (isempty())
    throw std::out_of_range("front() on empty list");
  return m_head->m_data;
}

template <typename T> const T &LinkedList<T>::front() const {
  if (isempty())
    throw std::out_of_range("front() on empty list");
  return m_head->m_data;
}

template <typename T> T &LinkedList<T>::at(size_t pos) {
  if (pos >= m_size)
    throw std::out_of_range("Index out of range");
  Node *current = m_head;
  for (size_t i = 0; i < pos; ++i) {
    current = current->m_next;
  }
  return current->m_data;
}

template <typename T> const T &LinkedList<T>::at(size_t pos) const {
  if (pos >= m_size)
    throw std::out_of_range("Index out of range");
  Node *current = m_head;
  for (size_t i = 0; i < pos; ++i) {
    current = current->m_next;
  }
  return current->m_data;
}

template <typename T> T &LinkedList<T>::back() {
  if (isempty())
    throw std::out_of_range("back() on empty list");
  // Traverse from head to find the last node
  Node *current = m_head;
  while (current->m_next != nullptr) {
    current = current->m_next;
  }
  return current->m_data;
}

template <typename T> const T &LinkedList<T>::back() const {
  if (isempty())
    throw std::out_of_range("back() on empty list");
  Node *current = m_head;
  while (current->m_next != nullptr) {
    current = current->m_next;
  }
  return current->m_data;
}

// --- Modifiers ---

template <typename T> void LinkedList<T>::push_front(T data) {
  Node *newNode = new Node(data);
  newNode->m_next = m_head;
  m_head = newNode;
  m_size++;
}

template <typename T> void LinkedList<T>::push_back(T data) {
  if (isempty()) {
    push_front(data);
    return;
  }
  // Traverse from head to find the last node
  Node *current = m_head;
  while (current->m_next != nullptr) {
    current = current->m_next;
  }
  current->m_next = new Node(data);
  m_size++;
}

template <typename T> void LinkedList<T>::insert_at(T data, size_t pos) {
  if (pos > m_size)
    throw std::out_of_range("Cannot insert out of range");
  if (pos == 0) {
    push_front(data);
    return;
  }
  if (pos == m_size) {
    push_back(data);
    return;
  }

  Node *prev = m_head;
  for (size_t i = 0; i < pos - 1; ++i) {
    prev = prev->m_next;
  }
  Node *newNode = new Node(data);
  newNode->m_next = prev->m_next;
  prev->m_next = newNode;
  m_size++;
}

template <typename T> void LinkedList<T>::pop_front() {
  if (isempty())
    throw std::out_of_range("pop_front() on empty list");
  Node *temp = m_head;
  m_head = m_head->m_next;
  delete temp;
  m_size--;
}

template <typename T> void LinkedList<T>::pop_back() {
  if (isempty())
    throw std::out_of_range("pop_back() on empty list");
  if (m_size == 1) {
    pop_front();
    return;
  }

  // Traverse from head to find the second-to-last node
  Node *prev = m_head;
  while (prev->m_next->m_next != nullptr) {
    prev = prev->m_next;
  }
  delete prev->m_next;
  prev->m_next = nullptr;
  m_size--;
}

template <typename T> void LinkedList<T>::pop_at(size_t pos) {
  if (pos >= m_size)
    throw std::out_of_range("Cannot pop out of range");
  if (pos == 0) {
    pop_front();
    return;
  }

  Node *prev = m_head;
  for (size_t i = 0; i < pos - 1; ++i) {
    prev = prev->m_next;
  }
  Node *toDelete = prev->m_next;
  prev->m_next = toDelete->m_next;
  delete toDelete;
  m_size--;
}

template <typename T> void LinkedList<T>::pop_val(T val) {
  size_t pos = find(val);
  if (pos < m_size) { // Only pop if found
    pop_at(pos);
  }
}

template <typename T> void LinkedList<T>::reverse() {
  if (m_size < 2)
    return;
  Node *prev = nullptr, *current = m_head, *next = nullptr;
  while (current != nullptr) {
    next = current->m_next;
    current->m_next = prev;
    prev = current;
    current = next;
  }
  m_head = prev;
}

// --- Utility ---

template <typename T> size_t LinkedList<T>::find(T val) const {
  Node *current = m_head;
  size_t index = 0;
  while (current != nullptr) {
    if (current->m_data == val)
      return index;
    current = current->m_next;
    index++;
  }
  return m_size; // Convention for "not found"
}

template <typename T> void LinkedList<T>::printlist() const {
  Node *current = m_head;
  while (current != nullptr) {
    std::cout << current->m_data;
    if (current->m_next != nullptr)
      std::cout << " -> ";
    current = current->m_next;
  }
}

template <typename T> size_t LinkedList<T>::size() const { return m_size; }

template <typename T> bool LinkedList<T>::isempty() const {
  return m_head == nullptr;
}

template <typename T>
std::ostream &operator<<(std::ostream &os, const LinkedList<T> &list) {
  list.printlist();
  return os;
}

template <typename T>

void LinkedList<T>::sort() {
  // The main head pointer is updated to the new head of the sorted list
  m_head = mergeSort(m_head);
}

template <typename T>

typename LinkedList<T>::Node *LinkedList<T>::mergeSort(Node *head) {
  // Base case: a list with 0 or 1 element is already sorted
  if (!head || !head->m_next) {
    return head;
  }

  // 1. SPLIT the list into two halves
  Node *middle = getMiddle(head);
  Node *right_half_head = middle->m_next;
  middle->m_next = nullptr; // Sever the link between the two halves

  // 2. RECURSE on both halves
  Node *sorted_left = mergeSort(head);
  Node *sorted_right = mergeSort(right_half_head);

  // 3. MERGE the sorted halves
  return sortedMerge(sorted_left, sorted_right);
}
template <typename T>
typename LinkedList<T>::Node *LinkedList<T>::getMiddle(Node *head) {
  if (!head) {
    return head;
  }

  Node *slow = head, *fast = head;
  // Move fast pointer by 2 and slow pointer by 1
  // When fast reaches the end, slow will be at the middle
  while (fast->m_next != nullptr && fast->m_next->m_next != nullptr) {
    slow = slow->m_next;
    fast = fast->m_next->m_next;
  }
  return slow;
}

template <typename T>
typename LinkedList<T>::Node *LinkedList<T>::sortedMerge(Node *left,
                                                         Node *right) {
  Node *result = nullptr;

  // Base cases
  if (!left)
    return right;
  if (!right)
    return left;

  // Pick either left or right, and recurse
  if (left->m_data <= right->m_data) {
    result = left;
    result->m_next = sortedMerge(left->m_next, right);
  } else {
    result = right;
    result->m_next = sortedMerge(left, right->m_next);
  }
  return result;
}

int main() {
  LinkedList<int> list;

  // Stress test 1: push_back 10,000 elements
  for (int i = 0; i < 10000; ++i) {
    list.push_back(i);
  }
  assert(list.size() == 10000);
  assert(list.front() == 0);
  assert(list.back() == 9999);

  // Stress test 2: Insert at different positions (front, middle, end)
  list.insert_at(-1, 0);           // insert at front
  list.insert_at(-2, 5000);        // insert in middle
  list.insert_at(-3, list.size()); // insert at end
  assert(list.size() == 10003);
  assert(list.at(0) == -1);
  assert(list.at(5000) == -2);
  assert(list.back() == -3);

  // Stress test 3: pop_front many times
  for (int i = 0; i < 5000; ++i) {
    list.pop_front();
  }
  assert(list.size() == 5003);

  // Stress test 4: Reverse list
  list.reverse();
  assert(list.front() == -3); // last became front after reverse

  // Stress test 5: pop_val repeatedly removing all -3
  list.push_back(-3);
  list.push_front(-3);
  size_t found_pos = list.find(-3);
  while (found_pos < list.size()) {
    list.pop_val(-3);
    found_pos = list.find(-3);
  }

  // Print final size and first 10 elements
  std::cout << "Final size: " << list.size() << std::endl;
  int count = 0;
  for (size_t i = 0; i < list.size() && count < 10; ++i) {
    std::cout << list.at(i) << " ";
    ++count;
  }
  std::cout << std::endl;
  list.reverse();
  list.sort();
  std::cout << list;
  std::cout << "Stress test completed successfully." << std::endl;

  return 0;
}
