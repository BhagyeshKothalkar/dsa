#include <iostream>
#include <stdexcept>
#include <vector>

template <typename T> class MaxHeap {
private:
  T *m_arr;       // Pointer to the dynamic array
  int m_capacity; // Maximum possible size of the heap
  int m_size;     // Current number of elements in the heap

  // Helper functions for array indexing (using 0-based indexing)
  int parent(int i) { return (i - 1) / 2; }
  int left(int i) { return (2 * i) + 1; }
  int right(int i) { return (2 * i) + 2; }

  // Helper functions to restore the heap property
  void heapifyUp(int index);   //  Moves a node up
  void heapifyDown(int index); // Moves a node down

public:
  // Constructor and Destructor
  explicit MaxHeap(int capacity);
  ~MaxHeap();

  // Core Heap Operations
  void insert(const T &key);
  T extractMax();
  T peekMax() const;

  // Utility Functions
  int getSize() const { return m_size; }
  bool isEmpty() const { return m_size == 0; }
  bool isFull() const { return m_size == m_capacity; }
  void printHeap() const; // For debugging
};

template <typename T>
MaxHeap<T>::MaxHeap(int capacity) : m_capacity(capacity), m_size(0) {
  if (capacity <= 0) {
    throw std::invalid_argument("Capacity must be positive.");
  }
  m_arr = new T[m_capacity];
}

template <typename T> MaxHeap<T>::~MaxHeap() { delete[] m_arr; }

template <typename T> void MaxHeap<T>::insert(const T &key) {
  if (isFull()) {
    throw std::out_of_range("Heap is full. Cannot insert.");
  }

  // Insert the new key at the end
  m_arr[m_size] = key;
  m_size++;

  // Fix the max-heap property by bubbling up from the last element
  heapifyUp(m_size - 1);
}

template <typename T> void MaxHeap<T>::heapifyUp(int index) {
  // While the node is not the root and is greater than its parent
  while (index != 0 && m_arr[index] > m_arr[parent(index)]) {
    std::swap(m_arr[index], m_arr[parent(index)]);
    index = parent(index); // Move up to the parent's index
  }
}

template <typename T> T MaxHeap<T>::extractMax() {
  if (isEmpty()) {
    throw std::out_of_range("Heap is empty. Cannot extract max.");
  }

  // The root holds the maximum element
  T maxElement = m_arr[0];

  // Replace the root with the last element
  m_arr[0] = m_arr[m_size - 1];
  m_size--;

  // Restore the heap property by sifting down from the root
  heapifyDown(0);

  return maxElement;
}

template <typename T> void MaxHeap<T>::heapifyDown(int index) {
  int largest = index;
  int left = left(index);
  int right = right(index);

  // Check if left child exists and is greater than the current largest
  if (left < m_size && m_arr[left] > m_arr[largest]) {
    largest = left;
  }

  // Check if right child exists and is greater than the current largest
  if (right < m_size && m_arr[right] > m_arr[largest]) {
    largest = right;
  }

  // If the largest element is not the current node, swap and recurse
  if (largest != index) {
    std::swap(m_arr[index], m_arr[largest]);
    // Continue to heapify down from the new position of the element
    heapifyDown(largest);
  }
}

template <typename T> T MaxHeap<T>::peekMax() const {
  if (isEmpty()) {
    throw std::out_of_range("Heap is empty.");
  }
  return m_arr[0];
}

template <typename T> void MaxHeap<T>::printHeap() const {
  std::cout << "Heap array: ";
  for (int i = 0; i < m_size; ++i) {
    std::cout << m_arr[i] << " ";
  }
  std::cout << std::endl;
}

int main() {
  std::cout << "Creating a MaxHeap with capacity 10." << std::endl;
  MaxHeap<int> heap(10);

  std::cout << "Inserting elements: 10, 20, 15, 30, 5, 40, 25" << std::endl;
  heap.insert(10);
  heap.insert(20);
  heap.insert(15);
  heap.insert(30);
  heap.insert(5);
  heap.insert(40);
  heap.insert(25);

  heap.printHeap(); // Print the internal array representation
  std::cout << "Current size: " << heap.getSize() << std::endl;
  std::cout << "Max element is: " << heap.peekMax() << std::endl;

  std::cout << "\nExtracting elements (Heap Sort in action):" << std::endl;
  while (!heap.isEmpty()) {
    std::cout << heap.extractMax() << " ";
  }
  std::cout << std::endl;

  std::cout << "Current size after extraction: " << heap.getSize() << std::endl;

  return 0;
}