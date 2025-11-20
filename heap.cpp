#include <iostream>
#include <stdexcept>
#include <vector>

template <typename T> class MaxHeap {
private:
T *m_arr;
int m_capacity;
int m_size;

int parent(int i) { return (i - 1) / 2; }
int left(int i) { return (2 * i) + 1; }
int right(int i) { return (2 * i) + 2; }

void heapifyUp(int index);
void heapifyDown(int index);

public:
explicit MaxHeap(int capacity);
~MaxHeap();

void insert(const T &key);
T extractMax();
T peekMax() const;

int getSize() const { return m_size; }
bool isEmpty() const { return m_size == 0; }
bool isFull() const { return m_size == m_capacity; }
void printHeap() const;
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

m_arr[m_size] = key;
m_size++;

heapifyUp(m_size - 1);
}

template <typename T> void MaxHeap<T>::heapifyUp(int index) {
while (index != 0 && m_arr[index] > m_arr[parent(index)]) {
std::swap(m_arr[index], m_arr[parent(index)]);
index = parent(index);
}
}

template <typename T> T MaxHeap<T>::extractMax() {
if (isEmpty()) {
throw std::out_of_range("Heap is empty. Cannot extract max.");
}

T maxElement = m_arr[0];

m_arr[0] = m_arr[m_size - 1];
m_size--;

heapifyDown(0);

return maxElement;
}

template <typename T> void MaxHeap<T>::heapifyDown(int index) {
int largest = index;
int left = left(index);
int right = right(index);

if (left < m_size && m_arr[left] > m_arr[largest]) {
largest = left;
}

if (right < m_size && m_arr[right] > m_arr[largest]) {
largest = right;
}

if (largest != index) {
std::swap(m_arr[index], m_arr[largest]);
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

heap.printHeap();
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
