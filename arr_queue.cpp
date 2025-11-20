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




