#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>

template <typename T> class DoublyLinkedList;

template <typename T>
std::ostream &operator<<(std::ostream &os, const DoublyLinkedList<T> &list);

template <typename T> class DoublyLinkedList {
private:
struct Node {
T m_data;
Node *m_next;
Node *m_prev;
Node(T data) : m_data(data), m_next(nullptr), m_prev(nullptr) {}
};

Node *m_head;
Node *m_tail;
size_t m_size;

Node *mergeSort(Node *head);
Node *getMiddle(Node *head);
Node *sortedMerge(Node *left, Node *right);
void updatePrevPointersAndTail();

public:
DoublyLinkedList();
~DoublyLinkedList();
DoublyLinkedList(const DoublyLinkedList &other);
DoublyLinkedList<T> &operator=(const DoublyLinkedList &other);

T &front();
const T &front() const;
T &at(size_t pos);
const T &at(size_t pos) const;
T &back();
const T &back() const;

void push_back(T data);
void push_front(T data);
void insert_at(T data, size_t pos);
void pop_front();
void pop_back();
void pop_at(size_t pos);
void pop_val(T val);
void reverse();

size_t find(T val) const;
size_t size() const;
bool isempty() const;
void sort();

friend std::ostream &operator<< <>(std::ostream &os,
const DoublyLinkedList<T> &list);
};

template <typename T>
DoublyLinkedList<T>::DoublyLinkedList()
: m_head(nullptr), m_tail(nullptr), m_size(0) {}

template <typename T> DoublyLinkedList<T>::~DoublyLinkedList() {
while (!isempty()) {
pop_front();
}
}

template <typename T>
DoublyLinkedList<T>::DoublyLinkedList(const DoublyLinkedList &other)
: m_head(nullptr), m_tail(nullptr), m_size(0) {
for (Node *current = other.m_head; current != nullptr;
current = current->m_next) {
push_back(current->m_data);
}
}

template <typename T>
DoublyLinkedList<T> &DoublyLinkedList<T>::operator=(const DoublyLinkedList &other) {
if (this == &other) {
return *this;
}
while (!isempty()) {
pop_front();
}
for (Node *current = other.m_head; current != nullptr;
current = current->m_next) {
push_back(current->m_data);
}
return *this;
}

template <typename T> T &DoublyLinkedList<T>::front() {
if (isempty()) throw std::out_of_range("front() on empty list");
return m_head->m_data;
}
template <typename T> const T &DoublyLinkedList<T>::front() const {
if (isempty()) throw std::out_of_range("front() on empty list");
return m_head->m_data;
}

template <typename T> T &DoublyLinkedList<T>::back() {
if (isempty()) throw std::out_of_range("back() on empty list");
return m_tail->m_data;
}
template <typename T> const T &DoublyLinkedList<T>::back() const {
if (isempty()) throw std::out_of_range("back() on empty list");
return m_tail->m_data;
}

template <typename T> T &DoublyLinkedList<T>::at(size_t pos) {
if (pos >= m_size) throw std::out_of_range("Index out of range");

Node *current;
if (pos < m_size / 2) {
current = m_head;
for (size_t i = 0; i < pos; ++i) {
current = current->m_next;
}
} else {
current = m_tail;
for (size_t i = 0; i < m_size - 1 - pos; ++i) {
current = current->m_prev;
}
}
return current->m_data;
}

template <typename T> const T &DoublyLinkedList<T>::at(size_t pos) const {
if (pos >= m_size) throw std::out_of_range("Index out of range");
return const_cast<DoublyLinkedList<T>*>(this)->at(pos);
}

template <typename T> void DoublyLinkedList<T>::push_front(T data) {
Node *newNode = new Node(data);
newNode->m_next = m_head;
if (m_head != nullptr) {
m_head->m_prev = newNode;
}
m_head = newNode;
if (m_tail == nullptr) {
m_tail = m_head;
}
m_size++;
}

template <typename T> void DoublyLinkedList<T>::push_back(T data) {
if (isempty()) {
push_front(data);
return;
}
Node *newNode = new Node(data);
newNode->m_prev = m_tail;
m_tail->m_next = newNode;
m_tail = newNode;
m_size++;
}

template <typename T> void DoublyLinkedList<T>::insert_at(T data, size_t pos) {
if (pos > m_size) throw std::out_of_range("Cannot insert out of range");
if (pos == 0) { push_front(data); return; }
if (pos == m_size) { push_back(data); return; }

Node *current = m_head;
for (size_t i = 0; i < pos; ++i) {
current = current->m_next;
}
Node *prev_node = current->m_prev;
Node *newNode = new Node(data);

newNode->m_next = current;
newNode->m_prev = prev_node;
prev_node->m_next = newNode;
current->m_prev = newNode;
m_size++;
}

template <typename T> void DoublyLinkedList<T>::pop_front() {
if (isempty()) throw std::out_of_range("pop_front() on empty list");
Node *temp = m_head;
m_head = m_head->m_next;
if (m_head != nullptr) {
m_head->m_prev = nullptr;
} else {
m_tail = nullptr;
}
delete temp;
m_size--;
}

template <typename T> void DoublyLinkedList<T>::pop_back() {
if (isempty()) throw std::out_of_range("pop_back() on empty list");
if (m_size == 1) { pop_front(); return; }

Node *temp = m_tail;
m_tail = m_tail->m_prev;
m_tail->m_next = nullptr;
delete temp;
m_size--;
}

template <typename T> void DoublyLinkedList<T>::pop_at(size_t pos) {
if (pos >= m_size) throw std::out_of_range("Cannot pop out of range");
if (pos == 0) { pop_front(); return; }
if (pos == m_size - 1) { pop_back(); return; }

Node *toDelete = m_head;
for (size_t i = 0; i < pos; ++i) {
toDelete = toDelete->m_next;
}
Node *prev_node = toDelete->m_prev;
Node *next_node = toDelete->m_next;
prev_node->m_next = next_node;
next_node->m_prev = prev_node;
delete toDelete;
m_size--;
}

template <typename T> void DoublyLinkedList<T>::pop_val(T val) {
size_t pos = find(val);
if (pos < m_size) {
pop_at(pos);
}
}

template <typename T> void DoublyLinkedList<T>::reverse() {
if (m_size < 2) return;
Node *current = m_head;
Node *temp_prev = nullptr;
while (current != nullptr) {
temp_prev = current->m_prev;
current->m_prev = current->m_next;
current->m_next = temp_prev;
current = current->m_prev;
}
temp_prev = m_head;
m_head = m_tail;
m_tail = temp_prev;
}

template <typename T> size_t DoublyLinkedList<T>::find(T val) const {
Node *current = m_head;
size_t index = 0;
while (current != nullptr) {
if (current->m_data == val) return index;
current = current->m_next;
index++;
}
return m_size;
}

template <typename T> size_t DoublyLinkedList<T>::size() const { return m_size; }
template <typename T> bool DoublyLinkedList<T>::isempty() const { return m_head == nullptr; }

template <typename T>
std::ostream &operator<<(std::ostream &os, const DoublyLinkedList<T> &list) {
typename DoublyLinkedList<T>::Node *current = list.m_head;
os << "[";
while (current != nullptr) {
os << current->m_data;
if (current->m_next != nullptr) os << ", ";
current = current->m_next;
}
os << "]";
return os;
}

template <typename T> void DoublyLinkedList<T>::sort() {
m_head = mergeSort(m_head);
updatePrevPointersAndTail();
}

template <typename T>
void DoublyLinkedList<T>::updatePrevPointersAndTail() {
if(isempty()) return;
Node* current = m_head;
current->m_prev = nullptr;
while(current->m_next != nullptr) {
current->m_next->m_prev = current;
current = current->m_next;
}
m_tail = current;
}

template <typename T>
typename DoublyLinkedList<T>::Node *DoublyLinkedList<T>::mergeSort(Node *head) {
if (!head || !head->m_next) return head;
Node *middle = getMiddle(head);
Node *right_half_head = middle->m_next;
middle->m_next = nullptr;
Node *sorted_left = mergeSort(head);
Node *sorted_right = mergeSort(right_half_head);
return sortedMerge(sorted_left, sorted_right);
}

template <typename T>
typename DoublyLinkedList<T>::Node *DoublyLinkedList<T>::getMiddle(Node *head) {
if (!head) return head;
Node *slow = head, *fast = head;
while (fast->m_next != nullptr && fast->m_next->m_next != nullptr) {
slow = slow->m_next;
fast = fast->m_next->m_next;
}
return slow;
}

template <typename T>
typename DoublyLinkedList<T>::Node *
DoublyLinkedList<T>::sortedMerge(Node *left, Node *right) {
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

int main() {
DoublyLinkedList<int> list;

for (int i = 0; i < 5000; ++i) {
list.push_back(i);
list.push_front(i * -1);
}
assert(list.size() == 10000);
assert(list.front() == -4999);
assert(list.back() == 4999);
std::cout << "Test 1 Passed: push_back and push_front OK." << std::endl;

for (int i = 0; i < 2000; ++i) {
list.pop_back();
list.pop_front();
}
assert(list.size() == 6000);
assert(list.front() == -2999);
assert(list.back() == 2999);
std::cout << "Test 2 Passed: pop_back and pop_front OK." << std::endl;

list.insert_at(9999, list.size());
list.insert_at(-9999, 0);
list.insert_at(0, 3000);
assert(list.size() == 6003);
assert(list.front() == -9999);
assert(list.at(3000) == 0);
assert(list.back() == 9999);
std::cout << "Test 3 Passed: insert_at and at() OK." << std::endl;

size_t pos_of_zero = list.find(0);
list.pop_at(pos_of_zero);
list.pop_at(0);
list.pop_at(list.size() - 1);
assert(list.size() == 6000);
assert(list.find(0) == list.size());
std::cout << "Test 4 Passed: pop_at and find() OK." << std::endl;

int old_front = list.front();
int old_back = list.back();
list.reverse();
assert(list.front() == old_back);
assert(list.back() == old_front);
list.reverse();
assert(list.front() == old_front);
assert(list.back() == old_back);
std::cout << "Test 5 Passed: reverse() OK." << std::endl;

list.sort();
assert(list.front() == -2999);
assert(list.back() == 2999);
for(size_t i = 0; i < 10; ++i) {
assert(list.at(i) <= list.at(i+1));
}
std::cout << "Test 6 Passed: sort() OK." << std::endl;

std::cout << "\nFinal list state (first 20 elements): ";
for (size_t i = 0; i < 20; ++i) {
std::cout << list.at(i) << " ";
}
std::cout << "..." << std::endl;

std::cout << "\nAll stress tests completed successfully." << std::endl;

return 0;
}
