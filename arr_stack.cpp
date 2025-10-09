#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string> // For testing with strings

// Use std:: prefixes instead of `using namespace std;` in header-like files
// FIX: Converted to a template class
template <typename T>
class ArrayStack {
private:
    T* m_arr;
    std::size_t m_max_size;
    std::size_t m_top; // Represents the index of the next available slot

public:
    // --- Constructors and Destructor ---
    explicit ArrayStack(std::size_t max_size = 16); // Default capacity
    ~ArrayStack();

    // --- Rule of Three: Essential for memory safety ---
    ArrayStack(const ArrayStack& other); // Copy Constructor
    ArrayStack& operator=(const ArrayStack& other); // Copy Assignment Operator

    // --- Core Stack Operations ---
    void push(const T& data);
    T pop();
    T peek() const; // FIX: Added const

    // --- Utility Methods ---
    bool isEmpty() const; // FIX: Added const
    bool isFull() const;  // FIX: Added const
    std::size_t size() const; // Good practice to have a size method
    void printstack() const; // FIX: Added const
};

// --- Implementations ---

// Constructor with a default capacity
template <typename T>
ArrayStack<T>::ArrayStack(std::size_t max_size) : m_max_size(max_size), m_top(0) {
    // FIX: Check for zero capacity to avoid useless allocations
    if (max_size == 0) {
        throw std::invalid_argument("Stack max_size cannot be zero.");
    }
    m_arr = new T[m_max_size];
}

// Destructor
template <typename T>
ArrayStack<T>::~ArrayStack() {
    delete[] m_arr;
}

// FIX: Copy Constructor (Deep Copy)
template <typename T>
ArrayStack<T>::ArrayStack(const ArrayStack& other)
    : m_max_size(other.m_max_size), m_top(other.m_top) {
    m_arr = new T[m_max_size];
    // Copy the contents of the other array, not just the pointer
    for (std::size_t i = 0; i < m_top; ++i) {
        m_arr[i] = other.m_arr[i];
    }
}

// FIX: Copy Assignment Operator (Deep Copy)
template <typename T>
ArrayStack<T>& ArrayStack<T>::operator=(const ArrayStack& other) {
    if (this == &other) { // Handle self-assignment
        return *this;
    }
    // Free the old memory before allocating new memory
    delete[] m_arr;
    
    m_max_size = other.m_max_size;
    m_top = other.m_top;
    m_arr = new T[m_max_size];

    for (std::size_t i = 0; i < m_top; ++i) {
        m_arr[i] = other.m_arr[i];
    }
    return *this;
}

template <typename T>
void ArrayStack<T>::push(const T& data) {
    // FIX: Corrected the fullness check
    if (isFull()) {
        throw std::out_of_range("Stack is full, cannot push.");
    }
    m_arr[m_top] = data;
    m_top++;
}

template <typename T>
T ArrayStack<T>::pop() {
    if (isEmpty()) {
        throw std::out_of_range("Stack is empty, cannot pop.");
    }
    m_top--;
    return m_arr[m_top];
}

template <typename T>
T ArrayStack<T>::peek() const {
    // FIX: Added safety check for empty stack
    if (isEmpty()) {
        throw std::out_of_range("Stack is empty, cannot peek.");
    }
    return m_arr[m_top - 1];
}

template <typename T>
bool ArrayStack<T>::isEmpty() const {
    // FIX: Corrected empty condition
    return m_top == 0;
}

template <typename T>
bool ArrayStack<T>::isFull() const {
    return m_top == m_max_size;
}

template <typename T>
std::size_t ArrayStack<T>::size() const {
    return m_top;
}

template <typename T>
void ArrayStack<T>::printstack() const {
    std::cout << "Stack (bottom to top): ";
    for (std::size_t i = 0; i < m_top; i++) {
        std::cout << m_arr[i] << " ";
    }
    std::cout << "(size: " << size() << "/" << m_max_size << ")" << std::endl;
}

// --- Updated Testing Code ---
int main() {
    // Test with integers
    std::cout << "--- Testing with integers ---" << std::endl;
    ArrayStack<int> int_stack(5);
    int_stack.push(10);
    int_stack.push(20);
    int_stack.push(30);
    int_stack.printstack();

    std::cout << "Peek: " << int_stack.peek() << std::endl;
    std::cout << "Popped: " << int_stack.pop() << std::endl;
    int_stack.printstack();
    
    std::cout << "Is empty? " << std::boolalpha << int_stack.isEmpty() << std::endl;
    std::cout << "Is full? " << std::boolalpha << int_stack.isFull() << std::endl;

    // Test with strings
    std::cout << "\n--- Testing with strings ---" << std::endl;
    ArrayStack<std::string> string_stack(4);
    string_stack.push("hello");
    string_stack.push("world");
    string_stack.printstack();

    // Test copy constructor
    ArrayStack<std::string> copied_stack = string_stack;
    copied_stack.push("!");
    
    std::cout << "Original stack after copy was modified:" << std::endl;
    string_stack.printstack();
    std::cout << "Copied stack:" << std::endl;
    copied_stack.printstack();

    return 0;
}