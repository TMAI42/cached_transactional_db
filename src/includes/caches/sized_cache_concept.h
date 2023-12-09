//
// Created by tmai42 on 12/9/23.
//

#ifndef SIZED_CACHE_CONCEPT_H
#define SIZED_CACHE_CONCEPT_H

#include <concepts>
#include <cstddef>

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
