//
// Created by tmai42 on 12/8/23.
//

#ifndef FIRST_IN_FIRST_OUT_CACHE_H
#define FIRST_IN_FIRST_OUT_CACHE_H

#include <deque>
#include <iterator>
#include <mutex>
#include <unordered_map>
#include <utility>

template <typename Key, typename Value>
class first_if_first_out_cache {

public:
    using Value_type = typename std::pair<Key, Value>;
    using Value_itr = typename std::deque<Value_type>::iterator;

    explicit first_if_first_out_cache(size_t max_size)
    : max_cache_size(max_size ? max_size : std::numeric_limits<size_t>::max()) {}

    void set(const Key& key, const Value& value) {
        std::lock_guard<std::mutex> lock(mutex);

        auto cache_items_map_itr = cache_items_map.find(key);

        if (cache_items_map_itr == cache_items_map.end()) {
            if (cache_items_map.size() + 1 > max_cache_size) {
                // remove the last element from cache
                auto last = cache_items_deque.rbegin();

                cache_items_map.erase(last->first);
                cache_items_deque.pop_back();
            }

            cache_items_deque.push_front(std::make_pair(key, value));
            cache_items_map[key] = cache_items_deque.begin();
        }
        else {
            // just update value
            cache_items_map_itr->second->second = value;
        }
    }

    const Value& get(const Key& key) const {
        std::lock_guard<std::mutex> lock(mutex);

        const auto cache_items_map_itr = cache_items_map.find(key);

        if (cache_items_map_itr == cache_items_map.end()) {
            throw std::range_error("No such key in the cache");
        }

        return cache_items_map_itr->second->second;
    }

    bool is_cached(const Key& key) const noexcept {
        std::lock_guard<std::mutex> lock(mutex);
        return cache_items_map.find(key) != cache_items_map.end();
    }

    Value remove(const Key& key) noexcept {
        std::lock_guard<std::mutex> lock(mutex);

        const auto cache_items_map_itr = cache_items_map.find(key);

        if (cache_items_map_itr == cache_items_map.end()) {
            return Value{};
        }
         const Value removed = cache_items_map_itr->second->second;

        cache_items_deque.erase(cache_items_map_itr->second);
        cache_items_map.erase(cache_items_map_itr);


        return removed;
    }

private:
    std::deque<Value_type> cache_items_deque;
    std::unordered_map<Key, Value_itr> cache_items_map;
    size_t max_cache_size;
    mutable std::mutex mutex; // mutable because we want to lock does not affect const modifier for functions
};

#endif //FIRST_IN_FIRST_OUT_CACHE_H
