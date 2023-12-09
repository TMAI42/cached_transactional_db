//
// Created by tmai42 on 12/9/23.
//

#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <cstddef>
#include <list>
#include <mutex>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <limits>

template <typename Key, typename Value>
class lru_cache {

public:

    using value_type = typename std::pair<Key, Value>;
    using value_it = typename std::list<value_type>::iterator;

    explicit lru_cache(size_t max_size) : max_cache_size(max_size ? max_size : std::numeric_limits<size_t>::max()) {}

    void put(const Key& key, const Value& value) {
        std::lock_guard<std::mutex> lock(mutex);
        auto it = cache_items_map.find(key);

        if (it == cache_items_map.end()) {
            if (cache_items_map.size() + 1 > max_cache_size) {
                // remove the last element from cache
                auto last = cache_items_list.crbegin();

                cache_items_map.erase(last->first);
                cache_items_list.pop_back();
            }

            cache_items_list.push_front(std::make_pair(key, value));
            cache_items_map[key] = cache_items_list.begin();
        }
        else {
            it->second->second = value;
            cache_items_list.splice(cache_items_list.cbegin(), cache_items_list,
                                    it->second);
        }
    }

    const Value& get(const Key& key) const {
        std::lock_guard<std::mutex> lock(mutex);
        auto it = cache_items_map.find(key);

        if (it == cache_items_map.end()) {
            throw std::range_error("No such key in the cache");
        }
        else {
            cache_items_list.splice(cache_items_list.begin(), cache_items_list,
                                    it->second);

            return it->second->second;
        }
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

        cache_items_list.erase(cache_items_map_itr->second);
        cache_items_map.erase(cache_items_map_itr);

        return removed;
    }

private:
    mutable std::list<value_type> cache_items_list;
    std::unordered_map<Key, value_it> cache_items_map;
    size_t max_cache_size;
    mutable std::mutex mutex;
};

#endif //LRU_CACHE_H
