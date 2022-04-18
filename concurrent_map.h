#pragma once

#include <cstdlib>
#include <execution>
#include <future>
#include <map>
#include <numeric>
#include <random>
#include <string>
#include <vector>

template <typename Key, typename Value>
class ConcurrentMap {
public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys");

    explicit ConcurrentMap(size_t bucket_count) : buckets_count_(bucket_count), buckets_(buckets_count_) {}

    struct Bucket {
        mutable std::mutex m;
        std::map<Key, Value> container;
    };

    struct Access {
        Access(const Key& key, size_t index, std::vector<Bucket>& buckets)
            : guard(buckets[index].m), ref_to_value(buckets[index].container[key]) {}

        std::lock_guard<std::mutex> guard;
        Value& ref_to_value;
    };

    Access operator[](const Key& key) { return Access(key, GetIndex(key), buckets_); }

    std::map<Key, Value> BuildOrdinaryMap() {
        std::map<Key, Value> ordinary_map;

        for (const auto& bucket : buckets_) {
            std::lock_guard<std::mutex> guard(bucket.m);
            ordinary_map.insert(bucket.container.begin(), bucket.container.end());
        }

        return ordinary_map;
    }

    void Erase(const Key& key) {
        std::lock_guard<std::mutex> guard(buckets_[GetIndex(key)].m);
        buckets_[GetIndex(key)].container.erase(key);
    }

private:
    size_t GetIndex(const Key& key) { return static_cast<size_t>(key) % buckets_count_; }

private:
    size_t buckets_count_;
    std::vector<Bucket> buckets_;
};
