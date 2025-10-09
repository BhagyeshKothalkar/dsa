
#include <cmath>
#include <string>

#include <climits>
#include <cstddef>
#include <iostream>
#include <stdexcept>

using namespace std;

// Linked list
template <typename T> class LinkedList {
private:
  struct node {
    node();
    node(T data);
    T m_data;
    node *m_next;
  };
  node *m_head;
  size_t m_size;

private:
  size_t insert_val(node *to_insert, size_t sorted_size);

public:
  LinkedList();
  LinkedList(T data);
  T front();
  T at(size_t pos);
  T back();

  void push_back(T data);
  void push_front(T data);
  void insert_at(T data, size_t pos);

  void pop_front();
  void pop_back();
  void pop_at(size_t pos);

  size_t find(T val);
  void pop_val(T val);
  LinkedList *merge(LinkedList &other);
  void reverse();
  void sort();
  void printlist();

  size_t size();
  bool isempty();

  int max();
  int min();
};

template <typename T> LinkedList<T>::node::node() { m_next = nullptr; }

template <typename T> LinkedList<T>::node::node(T data) {
  m_data = data;
  m_next = nullptr;
}

template <typename T> LinkedList<T>::LinkedList() {
  m_head = nullptr;
  m_size = 0;
}

template <typename T> LinkedList<T>::LinkedList(T data) {
  m_head = new node(data);
  m_size = 1;
}

template <typename T> T LinkedList<T>::front() {
  if (m_head == NULL)
    throw out_of_range("attempting to get front element of an empty "
                       "LinkedList");
  return m_head->m_data;
}

template <typename T> T LinkedList<T>::at(size_t pos) {
  if (m_head == NULL)
    throw out_of_range("attempting to get front element of an empty "
                       "LinkedList");
  if (m_head->m_next == NULL) {
    if (pos == 0)
      return m_head->m_data;

    else
      throw out_of_range("attempting to get front element of an empty "
                         "LinkedList");
  } else {
    size_t cnt = 0;
    node *idx = m_head;
    while (idx->m_next != nullptr && cnt < pos) {
      idx = idx->m_next;
      cnt++;
    }
    return idx->m_data;
  }
}

template <typename T> T LinkedList<T>::back() {
  if (m_size == 0)
    throw out_of_range("attempting to get front element of an empty "
                       "LinkedList");

  return at(m_size - 1);
}

template <typename T>
void LinkedList<T>::push_front(T data)
// adds an element at the front
{
  node *temp = new node();
  temp->m_data = data;
  temp->m_next = m_head; // even if m_head is nullptr (list is empty), the
                         // assignment holds, without any modification
  m_head = temp;
}

template <typename T> void LinkedList<T>::push_back(T data) {
  // adds an element to the end of the LinkedList
  if (m_head == nullptr) {
    push_front(data);
  } else {
    node *temp = m_head;
    while (temp->m_next != nullptr) {
      temp = temp->m_next;
    }
    node *to_add = new node(data);
    temp->m_next = to_add;
  }
}

template <typename T> void LinkedList<T>::insert_at(T data, size_t pos) {
  // inserts an element at the specified index of the LinkedList
  if (m_head == nullptr && pos == 0) {
    push_front(data);
  } else if (m_head == nullptr && pos != 0) {
    throw out_of_range("attempting to access at" + to_string(pos) +
                       "location of an empty LinkedList");
  } else {
    node *temp = m_head;
    size_t cnt = 0;
    while (temp != nullptr && cnt + 1 != pos) {
      cnt++;
      temp = temp->m_next;
    }
    if (temp == nullptr)
      throw out_of_range("Position " + to_string(pos) +
                         "invalid for a LinkedList of size" +
                         to_string(cnt + 1));
    else {

      if (temp->m_next == nullptr) {
        push_back(data);
      }

      else {
        node *next = temp->m_next;
        node *to_add = new node();
        to_add->m_data = data;
        to_add->m_next = next;
        temp->m_next = to_add;
      }
    }
  }
}

template <typename T>
void LinkedList<T>::pop_front()
// removes the first element from the LinkedList
{
  if (m_head == nullptr) {
    throw out_of_range("pop_front to an empty LinkedList");
    return;
  }
  node *temp = m_head->m_next;
  delete m_head;
  m_head = temp;
}

template <typename T>
void LinkedList<T>::pop_back()
// removes the last element from the LinkedList<T>
{
  if (m_head == nullptr) {
    throw out_of_range("pop_back to an empty LinkedList");
  } else if (m_head->m_next == nullptr) {
    pop_front();
  } else {
    node *temp = m_head;
    while (temp->m_next->m_next != nullptr) {
      temp = temp->m_next;
    }
    delete temp->m_next;
    temp->m_next = nullptr;
  }
}

template <typename T>
void LinkedList<T>::pop_at(size_t pos)
// removes the element from the spacified position in the LinkedList
{
  if (m_head == nullptr) {
    throw out_of_range("pop_back to an empty LinkedList");
  } else if (pos == 0) {
    pop_front();
  } else {
    node *temp = m_head;
    size_t cnt = 0;
    while (temp != nullptr && cnt + 1 != pos) {
      cnt++;
      temp = temp->m_next;
    }
    if (temp == nullptr) {
      throw out_of_range("trying to delete " + to_string(pos) +
                         " th element of the LinkedList that contains " +
                         to_string(cnt + 1) + " elements.");
    } else {
      if (temp->m_next == nullptr)
        pop_back();
      else {
        node *next = temp->m_next->m_next;
        delete temp->m_next;
        temp->m_next = next;
      }
    }
  }
}

template <typename T> size_t LinkedList<T>::find(T val) {
  if (m_head == NULL)
    throw out_of_range("finding an element in an empty LinkedList");

  else {
    node *temp = m_head;
    size_t cnt = 0;
    while (temp != nullptr && temp->m_data != val) {
      cnt++;
      temp = temp->m_next;
    }
    if (temp == nullptr) {
      return m_size;
    }
    return cnt;
  }
}

template <typename T> void LinkedList<T>::pop_val(T val) {
  T idx = find(val);
  pop_at(idx);
}

template <typename T>
LinkedList<T> *LinkedList<T>::merge(LinkedList<T> &other) {
  node *temp = m_head;
  LinkedList<T> *newlist = new LinkedList<T>();
  while (temp != nullptr) {
    newlist->push_back(temp->m_data);
    temp = temp->m_next;
  }
  temp = other.m_head;
  while (temp != nullptr) {
    newlist->push_back(temp->m_data);
    temp = temp->m_next;
  }
  return newlist;
}

template <typename T> void LinkedList<T>::reverse() {
  node *curr = m_head->m_next->m_next, *prev = m_head;
  m_head = m_head->m_next;
  m_head->m_next = prev;
  prev->m_next = NULL;
  while (curr->m_next != NULL) {
    prev = m_head;
    m_head = curr;
    curr = curr->m_next;
    m_head->m_next = prev;
  }
  curr->m_next = m_head;
  m_head = curr;
}

template <typename T> void LinkedList<T>::printlist() {
  node *temp = m_head;
  size_t cnt = 0;
  while (temp != nullptr) {
    cout << temp->m_data;
    if (temp->m_next != nullptr)
      cout << "->";
    temp = temp->m_next;
    cnt++;
  }
  // cout << endl;
}

template <typename T> size_t LinkedList<T>::size() { return m_size; }

template <typename T> bool LinkedList<T>::isempty() {
  return m_head == nullptr;
}

// Stack using the Linked List

template <typename T> class StackUsingLL {
private:
  LinkedList<T> m_l;
  size_t m_size;

public:
  StackUsingLL();
  StackUsingLL(T *begin, T *end);

  void push(T data);
  void pop();
  T peek();

  bool isEmpty();
  void printstack();
};
template <typename T> class ListStack {
private:
  LinkedList<T> m_list; // Composition: the stack "has-a" linked list

public:
  // Pushes an element onto the top of the stack. (O(1))
  void push(const T &data) { m_list.push_front(data); }

  // Removes and returns the top element of the stack. (O(1))
  T pop() {
    if (isEmpty()) {
      throw std::out_of_range("Stack is empty, cannot pop.");
    }
    T val = m_list.front();
    m_list.pop_front();
    return val;
  }

  // Returns the top element without removing it. (O(1))
  T &peek() {
    if (isEmpty()) {
      throw std::out_of_range("Stack is empty, cannot peek.");
    }
    return m_list.front();
  }

  // Returns the number of elements in the stack.
  size_t size() const { return m_list.size(); }

  // Checks if the stack is empty.
  bool isEmpty() const { return m_list.isempty(); }
};