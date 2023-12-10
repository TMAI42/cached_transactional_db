//
// Created by tmai42 on 12/9/23.
//

#ifndef SIZED_CACHE_CONCEPT_H
#define SIZED_CACHE_CONCEPT_H

#include <concepts>
#include <cstddef>

/**
 * The sized_cache_concept concept defines the requirements for a cache class.
 *
 * Why concept is used instead of inheritance tree: We know class of cache in time of compilation, so there is no need to
 * add additional overhead of virtualization in this case
 *
 * @tparam T The cache type.
 * @tparam Key The type of the keys used to access values in the cache.
 * @tparam Value The type of the values stored in the cache.
 */
template<typename T, typename Key, typename Value> concept sized_cache_concept = requires(T t, const Key &key,
                                                                                          const Value &value,
                                                                                          size_t max_size) {
    { T(max_size) } -> std::same_as<T>;
    { t.put(key, value) };
    { t.get(key) } -> std::same_as<const Value &>;
    { t.remove(key) } -> std::same_as<Value>;
    { t.is_cached(key) } noexcept -> std::same_as<bool>;
};

#endif //SIZED_CACHE_CONCEPT_H
