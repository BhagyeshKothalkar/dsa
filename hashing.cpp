// open_addressing_maps.hpp
// C++17 - canonical open-addressing hash maps + hash function policies
//
// Usage: include and instantiate e.g.
//   LinearProbingHashMap<int, std::string, DivisionHash<int>> map;
//   map.insert(42, "hello");
//
// This file is self-contained.
#include <vector>
#include <optional>
#include <functional>
#include <stdexcept>
#include <utility>
#include <string>
#include <type_traits>
#include <limits>
#include <random>
#include <algorithm>
#include <cassert>


// -----------------------------
// Hash Policy interface (conceptual):
// struct HashPolicy {
//   using key_type = Key;
//   // compute primary hash in range [0, mod-1]
//   size_t operator()(const key_type& k, size_t mod) const;
//   // (optional) second hash used by double hashing; must be non-zero and < mod
//   size_t second(const key_type& k, size_t mod) const; // optional
// };
// -----------------------------

// -----------------------------
// Hash function policies
// -----------------------------

// Division method: h(k) = k mod M
template <typename Key>
struct DivisionHash {
    using key_type = Key;
    // For integral types only
    static_assert(std::is_integral<Key>::value, "DivisionHash requires integral Key");
    size_t operator()(Key k, size_t mod) const noexcept {
        return static_cast<size_t>( ( (k % static_cast<Key>(mod)) + static_cast<Key>(mod) ) % static_cast<Key>(mod) );
    }
    // second hash for double hashing (if needed): make odd
    size_t second(Key k, size_t mod) const noexcept {
        // return 1 + (k mod (mod-1))
        size_t m1 = (mod > 1 ? mod - 1 : 1);
        size_t r = static_cast<size_t>( ( (k % static_cast<Key>(m1)) + static_cast<Key>(m1) ) % static_cast<Key>(m1) );
        return 1 + r;
    }
};

// Multiplication method (Knuth). Works for integral keys.
template <typename Key>
struct MultiplicationHash {
    using key_type = Key;
    static_assert(std::is_integral<Key>::value, "MultiplicationHash requires integral Key");
    // A should be in (0,1). default uses Knuth's constant 2654435769 / 2^32 (approx)
    const uint64_t A;
    MultiplicationHash(uint64_t a = 2654435769ULL) : A(a) {}
    size_t operator()(Key k, size_t mod) const noexcept {
        // Use 64-bit mixing
        uint64_t x = static_cast<uint64_t>(k);
        uint64_t prod = x * A;
        // take high bits: prod >> (w - r) where w=64 and r = log2(mod) approx -> easier to do mod
        // canonical multiplication method simplified: (k * A) % 2^w then take (prod % mod)
        return static_cast<size_t>(prod % mod);
    }
    size_t second(Key k, size_t mod) const noexcept {
        // fallback to division-style secondary
        size_t m1 = (mod > 1 ? mod - 1 : 1);
        return 1 + static_cast<size_t>( (static_cast<uint64_t>(k) % m1) );
    }
};

// Mid-square for integer keys (classic)
template <typename Key>
struct MidSquareHash {
    using key_type = Key;
    static_assert(std::is_integral<Key>::value, "MidSquareHash requires integral Key");
    size_t operator()(Key k, size_t mod) const noexcept {
        // square and extract middle bits; produce result in [0, mod-1]
        uint64_t x = static_cast<uint64_t>(k);
        uint64_t sq = x * x;
        // take middle 32 bits of 64-bit square (shift by 16)
        uint64_t mid = (sq >> 16) & 0xffffffffULL;
        return static_cast<size_t>(mid % mod);
    }
    size_t second(Key k, size_t mod) const noexcept {
        // simple alternative
        return DivisionHash<Key>()(k + 1, mod - 1) + 1;
    }
};

// Polynomial rolling hash for strings using Horner's rule
struct PolynomialRollingHash {
    using key_type = std::string;
    const uint64_t base;
    const uint64_t modprime; // prime modulus for internal mixing (we still reduce mod table size)
    PolynomialRollingHash(uint64_t base_ = 257, uint64_t modprime_ = 1000000007ULL)
        : base(base_), modprime(modprime_) {}
    size_t operator()(const std::string& s, size_t mod) const noexcept {
        uint64_t r = 0;
        for (unsigned char c : s) {
            r = (r * base + static_cast<uint64_t>(c)) % modprime;
        }
        return static_cast<size_t>(r % mod);
    }
    // second hash: simple polynomial with different base
    size_t second(const std::string& s, size_t mod) const noexcept {
        uint64_t r = 0, otherBase = base ^ 1315423911u;
        for (unsigned char c : s) {
            r = (r * otherBase + static_cast<uint64_t>(c)) % modprime;
        }
        size_t m1 = (mod > 1 ? mod - 1 : 1);
        return 1 + static_cast<size_t>(r % m1);
    }
};

// Universal hashing family: h_{a,b}(k) = ((a*k + b) mod p) mod m
// Works for integral keys. User provides prime p > max(key).
template <typename Key>
struct UniversalHash {
    using key_type = Key;
    static_assert(std::is_integral<Key>::value, "UniversalHash requires integral Key");
    uint64_t a, b, p;
    UniversalHash(uint64_t a_, uint64_t b_, uint64_t p_ = 4294967311ULL) : a(a_), b(b_), p(p_) {}
    size_t operator()(Key k, size_t mod) const noexcept {
        uint64_t ak = ( (a % p) * ( static_cast<uint64_t>(k) % p ) ) % p;
        uint64_t val = (ak + (b % p)) % p;
        return static_cast<size_t>(val % mod);
    }
    size_t second(Key k, size_t mod) const noexcept {
        // derive a second hash from different params
        uint64_t ak = ( ((a ^ 0x9e3779b97f4a7c15ULL) % p) * ( static_cast<uint64_t>(k) % p ) ) % p;
        uint64_t val = (ak + ((b ^ 0x9e3779b97f4a7c15ULL) % p)) % p;
        size_t m1 = (mod > 1 ? mod - 1 : 1);
        return 1 + static_cast<size_t>(val % m1);
    }
};

// -----------------------------
// Utility: next prime for capacity sizing (simple)
inline bool is_prime(size_t n) {
    if (n < 2) return false;
    if (n % 2 == 0) return n == 2;
    for (size_t i = 3; i * i <= n; i += 2)
        if (n % i == 0) return false;
    return true;
}
inline size_t next_prime(size_t n) {
    while (!is_prime(n)) ++n;
    return n;
}

// -----------------------------
// Generic probing-map base helpers
// -----------------------------

enum class SlotState : uint8_t { Empty = 0, Occupied = 1, Deleted = 2 };

// A utility slot storing key/value and state
template <typename Key, typename Value>
struct Slot {
    SlotState state;
    Key key;
    Value value;
    Slot() : state(SlotState::Empty), key(), value() {}
};

// -----------------------------
// Linear probing map
// -----------------------------
template <
    typename Key,
    typename Value,
    typename HashPolicy = DivisionHash<Key>,
    typename KeyEqual = std::equal_to<Key>
>
class LinearProbingHashMap {
public:
    using key_type = Key;
    using mapped_type = Value;
    using hash_policy = HashPolicy;

    explicit LinearProbingHashMap(size_t initial_capacity = 16, double max_load = 0.6)
        : policy_(), eq_(), size_(0), deleted_count_(0), max_load_(max_load) {
        capacity_ = next_prime(std::max<size_t>(initial_capacity, 3));
        table_.assign(capacity_, Slot<Key,Value>());
    }

    bool insert(const Key& k, const Value& v) {
        if ((size_ + deleted_count_ + 1) > static_cast<size_t>(capacity_ * max_load_))
            rehash(next_prime(capacity_ * 2));

        size_t h = policy_(k, capacity_);
        for (size_t i = 0; i < capacity_; ++i) {
            size_t idx = (h + i) % capacity_;
            auto &slot = table_[idx];
            if (slot.state == SlotState::Empty) {
                slot.key = k;
                slot.value = v;
                slot.state = SlotState::Occupied;
                ++size_;
                return true;
            } else if (slot.state == SlotState::Deleted) {
                // reuse tombstone
                slot.key = k;
                slot.value = v;
                slot.state = SlotState::Occupied;
                ++size_;
                --deleted_count_;
                return true;
            } else if (slot.state == SlotState::Occupied && eq_(slot.key, k)) {
                // update
                slot.value = v;
                return false;
            }
        }
        // table full (shouldn't happen thanks to resize)
        rehash(next_prime(capacity_ * 2 + 1));
        return insert(k, v);
    }

    std::optional<Value> find(const Key& k) const {
        size_t h = policy_(k, capacity_);
        for (size_t i = 0; i < capacity_; ++i) {
            size_t idx = (h + i) % capacity_;
            const auto &slot = table_[idx];
            if (slot.state == SlotState::Empty) return std::nullopt;
            if (slot.state == SlotState::Occupied && eq_(slot.key, k)) return slot.value;
            // if deleted or occupied with different key, continue
        }
        return std::nullopt;
    }

    bool contains(const Key& k) const {
        return static_cast<bool>(find(k));
    }

    bool erase(const Key& k) {
        size_t h = policy_(k, capacity_);
        for (size_t i = 0; i < capacity_; ++i) {
            size_t idx = (h + i) % capacity_;
            auto &slot = table_[idx];
            if (slot.state == SlotState::Empty) return false;
            if (slot.state == SlotState::Occupied && eq_(slot.key, k)) {
                slot.state = SlotState::Deleted;
                // placement of key/value is optional; we leave them
                --size_;
                ++deleted_count_;
                return true;
            }
        }
        return false;
    }

    size_t size() const noexcept { return size_; }
    void clear() noexcept {
        table_.assign(capacity_, Slot<Key,Value>());
        size_ = 0;
        deleted_count_ = 0;
    }

    // Batch load vector of pairs
    void batch_load(const std::vector<std::pair<Key,Value>>& items, bool dedup = false) {
        if (dedup) {
            // naive dedup: sort by key and insert unique
            std::vector<std::pair<Key,Value>> tmp = items;
            std::sort(tmp.begin(), tmp.end(), [&](auto &a, auto &b){ return a.first < b.first; });
            tmp.erase(std::unique(tmp.begin(), tmp.end(), [](auto &a, auto &b){ return a.first == b.first; }), tmp.end());
            for (auto &kv : tmp) insert(kv.first, kv.second);
        } else {
            for (auto &kv : items) insert(kv.first, kv.second);
        }
    }

private:
    HashPolicy policy_;
    KeyEqual eq_;
    size_t capacity_;
    std::vector<Slot<Key,Value>> table_;
    size_t size_;
    size_t deleted_count_;
    double max_load_;

    void rehash(size_t new_cap) {
        new_cap = next_prime(std::max<size_t>(new_cap, 3));
        std::vector<Slot<Key,Value>> old = std::move(table_);
        table_.assign(new_cap, Slot<Key,Value>());
        size_t old_capacity = capacity_;
        capacity_ = new_cap;
        size_ = 0;
        deleted_count_ = 0;
        for (size_t i = 0; i < old_capacity; ++i) {
            if (old[i].state == SlotState::Occupied) {
                insert(old[i].key, old[i].value);
            }
        }
    }
};

// -----------------------------
// Quadratic probing map (hi = h + c1*i + c2*i^2)
// -----------------------------
template <
    typename Key,
    typename Value,
    typename HashPolicy = DivisionHash<Key>,
    typename KeyEqual = std::equal_to<Key>
>
class QuadraticProbingHashMap {
public:
    using key_type = Key;
    using mapped_type = Value;

    QuadraticProbingHashMap(size_t initial_capacity = 17, double max_load = 0.5, size_t c1 = 1, size_t c2 = 1)
        : policy_(), eq_(), size_(0), deleted_count_(0), c1_(c1), c2_(c2), max_load_(max_load) {
        // good practice: odd prime capacity helps quadratic sequences
        capacity_ = next_prime(std::max<size_t>(initial_capacity, 3));
        table_.assign(capacity_, Slot<Key,Value>());
    }

    bool insert(const Key& k, const Value& v) {
        if ((size_ + deleted_count_ + 1) > static_cast<size_t>(capacity_ * max_load_))
            rehash(next_prime(capacity_ * 2));

        size_t h = policy_(k, capacity_);
        for (size_t i = 0; i < capacity_; ++i) {
            size_t idx = (h + c1_*i + c2_*i*i) % capacity_;
            auto &slot = table_[idx];
            if (slot.state == SlotState::Empty) {
                slot.key = k; slot.value = v; slot.state = SlotState::Occupied; ++size_; return true;
            } else if (slot.state == SlotState::Deleted) {
                slot.key = k; slot.value = v; slot.state = SlotState::Occupied; ++size_; --deleted_count_; return true;
            } else if (slot.state == SlotState::Occupied && eq_(slot.key, k)) {
                slot.value = v; return false;
            }
        }
        rehash(next_prime(capacity_ * 2)); // should be rare
        return insert(k, v);
    }

    std::optional<Value> find(const Key& k) const {
        size_t h = policy_(k, capacity_);
        for (size_t i = 0; i < capacity_; ++i) {
            size_t idx = (h + c1_*i + c2_*i*i) % capacity_;
            const auto &slot = table_[idx];
            if (slot.state == SlotState::Empty) return std::nullopt;
            if (slot.state == SlotState::Occupied && eq_(slot.key, k)) return slot.value;
        }
        return std::nullopt;
    }

    bool contains(const Key& k) const { return static_cast<bool>(find(k)); }

    bool erase(const Key& k) {
        size_t h = policy_(k, capacity_);
        for (size_t i = 0; i < capacity_; ++i) {
            size_t idx = (h + c1_*i + c2_*i*i) % capacity_;
            auto &slot = table_[idx];
            if (slot.state == SlotState::Empty) return false;
            if (slot.state == SlotState::Occupied && eq_(slot.key, k)) {
                slot.state = SlotState::Deleted; --size_; ++deleted_count_; return true;
            }
        }
        return false;
    }

    size_t size() const noexcept { return size_; }
    void clear() noexcept { table_.assign(capacity_, Slot<Key,Value>()); size_ = 0; deleted_count_ = 0; }

    void batch_load(const std::vector<std::pair<Key,Value>>& items, bool dedup=false) {
        if (!dedup) { for (auto &kv : items) insert(kv.first, kv.second); return; }
        std::vector<std::pair<Key,Value>> tmp = items;
        std::sort(tmp.begin(), tmp.end(), [](auto &a, auto &b){ return a.first < b.first; });
        tmp.erase(std::unique(tmp.begin(), tmp.end(), [](auto &a, auto &b){ return a.first == b.first; }), tmp.end());
        for (auto &kv : tmp) insert(kv.first, kv.second);
    }

private:
    HashPolicy policy_;
    KeyEqual eq_;
    size_t capacity_;
    std::vector<Slot<Key,Value>> table_;
    size_t size_;
    size_t deleted_count_;
    size_t c1_, c2_;
    double max_load_;

    void rehash(size_t new_cap) {
        new_cap = next_prime(std::max<size_t>(new_cap, 3));
        std::vector<Slot<Key,Value>> old = std::move(table_);
        size_t oldcap = capacity_;
        capacity_ = new_cap;
        table_.assign(capacity_, Slot<Key,Value>());
        size_ = 0; deleted_count_ = 0;
        for (size_t i = 0; i < oldcap; ++i) if (old[i].state == SlotState::Occupied) insert(old[i].key, old[i].value);
    }
};

// -----------------------------
// Double Hashing map: hi = (h1 + i * h2) mod M
// -----------------------------
template <
    typename Key,
    typename Value,
    typename HashPolicy1 = DivisionHash<Key>,
    typename HashPolicy2 = DivisionHash<Key>,
    typename KeyEqual = std::equal_to<Key>
>
class DoubleHashingHashMap {
public:
    DoubleHashingHashMap(size_t initial_capacity = 17, double max_load = 0.6)
        : hp1_(), hp2_(), eq_(), size_(0), deleted_count_(0), max_load_(max_load) {
        capacity_ = next_prime(std::max<size_t>(initial_capacity, 3));
        table_.assign(capacity_, Slot<Key,Value>());
    }

    bool insert(const Key& k, const Value& v) {
        if ((size_ + deleted_count_ + 1) > static_cast<size_t>(capacity_ * max_load_))
            rehash(next_prime(capacity_ * 2));
        size_t h1 = hp1_(k, capacity_);
        size_t h2 = hp2_.second ? hp2_.second(k, capacity_) : hp2_(k, capacity_);
        if (h2 == 0) h2 = 1;
        for (size_t i = 0; i < capacity_; ++i) {
            size_t idx = (h1 + i * h2) % capacity_;
            auto &slot = table_[idx];
            if (slot.state == SlotState::Empty) {
                slot.key = k; slot.value = v; slot.state = SlotState::Occupied; ++size_; return true;
            } else if (slot.state == SlotState::Deleted) {
                slot.key = k; slot.value = v; slot.state = SlotState::Occupied; ++size_; --deleted_count_; return true;
            } else if (slot.state == SlotState::Occupied && eq_(slot.key, k)) {
                slot.value = v; return false;
            }
        }
        rehash(next_prime(capacity_ * 2));
        return insert(k, v);
    }

    std::optional<Value> find(const Key& k) const {
        size_t h1 = hp1_(k, capacity_);
        size_t h2 = hp2_.second ? hp2_.second(k, capacity_) : hp2_(k, capacity_);
        if (h2 == 0) h2 = 1;
        for (size_t i = 0; i < capacity_; ++i) {
            size_t idx = (h1 + i * h2) % capacity_;
            const auto &slot = table_[idx];
            if (slot.state == SlotState::Empty) return std::nullopt;
            if (slot.state == SlotState::Occupied && eq_(slot.key, k)) return slot.value;
        }
        return std::nullopt;
    }

    bool erase(const Key& k) {
        size_t h1 = hp1_(k, capacity_);
        size_t h2 = hp2_.second ? hp2_.second(k, capacity_) : hp2_(k, capacity_);
        if (h2 == 0) h2 = 1;
        for (size_t i = 0; i < capacity_; ++i) {
            size_t idx = (h1 + i * h2) % capacity_;
            auto &slot = table_[idx];
            if (slot.state == SlotState::Empty) return false;
            if (slot.state == SlotState::Occupied && eq_(slot.key, k)) {
                slot.state = SlotState::Deleted; --size_; ++deleted_count_; return true;
            }
        }
        return false;
    }

    bool contains(const Key& k) const { return static_cast<bool>(find(k)); }
    size_t size() const noexcept { return size_; }
    void clear() noexcept { table_.assign(capacity_, Slot<Key,Value>()); size_ = 0; deleted_count_ = 0; }

    void batch_load(const std::vector<std::pair<Key,Value>>& items, bool dedup=false) {
        if (!dedup) { for (auto &kv : items) insert(kv.first, kv.second); return; }
        std::vector<std::pair<Key,Value>> tmp = items;
        std::sort(tmp.begin(), tmp.end(), [](auto &a, auto &b){ return a.first < b.first; });
        tmp.erase(std::unique(tmp.begin(), tmp.end(), [](auto &a, auto &b){ return a.first == b.first; }), tmp.end());
        for (auto &kv : tmp) insert(kv.first, kv.second);
    }

private:
    HashPolicy1 hp1_;
    HashPolicy2 hp2_;
    KeyEqual eq_;
    size_t capacity_;
    std::vector<Slot<Key,Value>> table_;
    size_t size_;
    size_t deleted_count_;
    double max_load_;

    void rehash(size_t new_cap) {
        new_cap = next_prime(std::max<size_t>(new_cap, 3));
        auto old = std::move(table_);
        size_t oldcap = capacity_;
        capacity_ = new_cap;
        table_.assign(capacity_, Slot<Key,Value>());
        size_ = 0; deleted_count_ = 0;
        for (size_t i = 0; i < oldcap; ++i) if (old[i].state == SlotState::Occupied) insert(old[i].key, old[i].value);
    }
};

// -----------------------------
// Cuckoo hashing (two tables / two hash functions).
// canonical simple implementation with relocation limit and rehash.
// -----------------------------
template <
    typename Key,
    typename Value,
    typename Hash1 = DivisionHash<Key>,
    typename Hash2 = MidSquareHash<Key>,
    typename KeyEqual = std::equal_to<Key>
>
class CuckooHashMap {
public:
    CuckooHashMap(size_t initial_capacity = 16, double max_load = 0.5, size_t max_kicks = 500)
        : h1_(), h2_(), eq_(), max_load_(max_load), max_kicks_(max_kicks) {
        capacity_ = next_prime(std::max<size_t>(initial_capacity, 3));
        table1_.assign(capacity_, std::optional<std::pair<Key,Value>>());
        table2_.assign(capacity_, std::optional<std::pair<Key,Value>>());
        size_ = 0;
    }

    bool insert(const Key& k, const Value& v) {
        if ((size_ + 1) > static_cast<size_t>(2 * capacity_ * max_load_)) {
            rehash(next_prime(capacity_ * 2));
        }
        if (contains(k)) return false; // no duplicates

        std::pair<Key,Value> cur = {k, v};
        size_t table_id = 1;
        for (size_t kick = 0; kick < max_kicks_; ++kick) {
            if (table_id == 1) {
                size_t idx = h1_(cur.first, capacity_);
                if (!table1_[idx]) {
                    table1_[idx] = cur; ++size_; return true;
                } else {
                    // evict
                    std::swap(cur, *table1_[idx]);
                    table_id = 2;
                }
            } else {
                size_t idx = h2_(cur.first, capacity_);
                if (!table2_[idx]) {
                    table2_[idx] = cur; ++size_; return true;
                } else {
                    std::swap(cur, *table2_[idx]);
                    table_id = 1;
                }
            }
        }
        // too many kicks -> rehash to larger size and retry
        rehash(next_prime(capacity_ * 2));
        return insert(cur.first, cur.second);
    }

    std::optional<Value> find(const Key& k) const {
        size_t i1 = h1_(k, capacity_);
        if (table1_[i1] && eq_(table1_[i1]->first, k)) return table1_[i1]->second;
        size_t i2 = h2_(k, capacity_);
        if (table2_[i2] && eq_(table2_[i2]->first, k)) return table2_[i2]->second;
        return std::nullopt;
    }

    bool contains(const Key& k) const { return static_cast<bool>(find(k)); }

    bool erase(const Key& k) {
        size_t i1 = h1_(k, capacity_);
        if (table1_[i1] && eq_(table1_[i1]->first, k)) { table1_[i1].reset(); --size_; return true; }
        size_t i2 = h2_(k, capacity_);
        if (table2_[i2] && eq_(table2_[i2]->first, k)) { table2_[i2].reset(); --size_; return true; }
        return false;
    }

    size_t size() const noexcept { return size_; }
    void clear() {
        table1_.assign(capacity_, std::nullopt);
        table2_.assign(capacity_, std::nullopt);
        size_ = 0;
    }

    void batch_load(const std::vector<std::pair<Key,Value>>& items, bool dedup=false) {
        if (!dedup) { for (auto &kv : items) insert(kv.first, kv.second); return; }
        std::vector<std::pair<Key,Value>> tmp = items;
        std::sort(tmp.begin(), tmp.end(), [](auto &a, auto &b){ return a.first < b.first; });
        tmp.erase(std::unique(tmp.begin(), tmp.end(), [](auto &a, auto &b){ return a.first == b.first; }), tmp.end());
        for (auto &kv : tmp) insert(kv.first, kv.second);
    }

private:
    Hash1 h1_;
    Hash2 h2_;
    KeyEqual eq_;
    size_t capacity_;
    std::vector<std::optional<std::pair<Key,Value>>> table1_;
    std::vector<std::optional<std::pair<Key,Value>>> table2_;
    size_t size_;
    double max_load_;
    size_t max_kicks_;

    void rehash(size_t new_cap) {
        new_cap = next_prime(std::max<size_t>(new_cap, 3));
        std::vector<std::pair<Key,Value>> items;
        items.reserve(size_);
        for (auto &o : table1_) if (o) items.push_back(*o);
        for (auto &o : table2_) if (o) items.push_back(*o);
        capacity_ = new_cap;
        table1_.assign(capacity_, std::nullopt);
        table2_.assign(capacity_, std::nullopt);
        size_ = 0;
        for (auto &kv : items) insert(kv.first, kv.second);
    }
};



template <
    typename Key,
    typename Value,
    typename H = std::hash<Key>,
    typename KeyEq = std::equal_to<Key>
>
class ChainingHashMap {
public:
    using key_type = Key;
    using mapped_type = Value;
    using value_type = std::pair<Key, Value>;
    using hasher = H;
    using key_equal = KeyEq;

    explicit ChainingHashMap(size_t initial_buckets = 16, double max_load = 0.75)
        : hash_(), eq_(), buckets_(std::max<size_t>(1, initial_buckets)),
          size_(0), max_load_factor_(max_load)
    {}

    // Insert or update. Returns true if a new element was inserted, false if an existing
    // element was updated.
    bool insert(const Key &k, const Value &v) {
        maybe_rehash_for_insert();
        size_t idx = bucket_index(k);
        auto &bucket = buckets_[idx];

        // linear scan in bucket using LinkedList.at(i)
        for (size_t i = 0, n = bucket.size(); i < n; ++i) {
            auto &kv = bucket.at(i);
            if (eq_(kv.first, k)) {
                kv.second = v; // update
                return false;
            }
        }

        // not found -> insert at back
        bucket.push_back(std::make_pair(k, v));
        ++size_;
        return true;
    }

    // Remove key; returns true if removed
    bool erase(const Key &k) {
        size_t idx = bucket_index(k);
        auto &bucket = buckets_[idx];
        for (size_t i = 0, n = bucket.size(); i < n; ++i) {
            auto &kv = bucket.at(i);
            if (eq_(kv.first, k)) {
                bucket.pop_at(i);
                --size_;
                return true;
            }
        }
        return false;
    }

    // Returns std::optional<Value> (copy) if found
    std::optional<Value> find(const Key &k) const {
        size_t idx = bucket_index(k);
        const auto &bucket = buckets_[idx];
        for (size_t i = 0, n = bucket.size(); i < n; ++i) {
            const auto &kv = bucket.at(i);
            if (eq_(kv.first, k)) return kv.second;
        }
        return std::nullopt;
    }

    bool contains(const Key &k) const { return static_cast<bool>(find(k)); }

    // operator[]: insert default-constructed value if missing, return reference
    Value &operator[](const Key &k) {
        maybe_rehash_for_insert();
        size_t idx = bucket_index(k);
        auto &bucket = buckets_[idx];
        for (size_t i = 0, n = bucket.size(); i < n; ++i) {
            auto &kv = bucket.at(i);
            if (eq_(kv.first, k)) return kv.second;
        }
        // not present -> insert default value and return ref to back()
        bucket.push_back(std::make_pair(k, Value()));
        ++size_;
        return bucket.back().second;
    }

    size_t size() const noexcept { return size_; }
    bool empty() const noexcept { return size_ == 0; }

    void clear() noexcept {
        for (auto &b : buckets_) {
            b.clear();
        }
        size_ = 0;
    }

    size_t bucket_count() const noexcept { return buckets_.size(); }

    double load_factor() const noexcept {
        return static_cast<double>(size_) / static_cast<double>(bucket_count());
    }

    double max_load_factor() const noexcept { return max_load_factor_; }
    void max_load_factor(double lf) {
        if (!(lf > 0.0)) throw std::invalid_argument("max_load_factor must be > 0");
        max_load_factor_ = lf;
    }

    // Batch load (vector of pairs) - optionally deduplicate by inserting in order (later inserts overwrite earlier)
    void batch_load(const std::vector<value_type> &items, bool dedup = false) {
        if (!dedup) {
            for (const auto &kv : items) insert(kv.first, kv.second);
        } else {
            // simple dedup: insert in sequence (last overwrite previous)
            for (const auto &kv : items) insert(kv.first, kv.second);
        }
    }

    // Rehash: explicitly change bucket count (will be adjusted to at least 1)
    void rehash(size_t new_bucket_count) {
        new_bucket_count = std::max<size_t>(1, new_bucket_count);
        std::vector<LinkedList<value_type>> new_buckets(new_bucket_count);

        // move elements
        for (auto &b : buckets_) {
            for (size_t i = 0, n = b.size(); i < n; ++i) {
                auto kv = b.at(i); // copy
                size_t idx = static_cast<size_t>(hash_(kv.first)) % new_bucket_count;
                new_buckets[idx].push_back(kv);
            }
        }
        buckets_.swap(new_buckets);
    }

private:
    hasher hash_;
    key_equal eq_;
    std::vector<LinkedList<value_type>> buckets_;
    size_t size_;
    double max_load_factor_;

    inline size_t bucket_index(const Key &k) const {
        return static_cast<size_t>(hash_(k)) % buckets_.size();
    }

    void maybe_rehash_for_insert() {
        if (load_factor() > max_load_factor_) {
            // double buckets
            rehash(buckets_.size() * 2);
        }
    }
};

