//
// Created by tmai42 on 12/8/23.
//

#ifndef TRANSACTIONAL_DATABASE_H
#define TRANSACTIONAL_DATABASE_H

#include "i_db.h"
#include "cashes/lru_cache.h"

#include <optional>
#include <map>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <concepts>

template<typename T, typename Key, typename Value> concept sized_cache_concept = requires(T t,
                                                                                          const Key &key,
                                                                                          const Value &value,
                                                                                          size_t max_size) {
    { T(max_size) } -> std::same_as<T>;
    { t.put(key, value) };
    { t.get(key) } -> std::same_as<const Value &>;
    { t.remove(key) } -> std::same_as<Value>;
    { t.is_cached(key) } noexcept -> std::same_as<bool>;
};

template<typename T = lru_cache<std::string, std::string> > requires sized_cache_concept<T, std::string, std::string>
class transactional_database : public i_db {
public:

    explicit transactional_database(size_t cache_size) : cache(cache_size) {}

    bool begin_transaction() override {
        std::lock_guard<std::mutex> lock(mutex);

        const std::thread::id this_thread_id = std::this_thread::get_id();

        // we assume that we have unique transaction for each thread,
        // since i_db could not give us any instruments to differentiate transactions in one thread
        if (transactions.contains(this_thread_id))
            return false;

        transactions[this_thread_id];
        return true;
    }

    bool commit_transaction() override {
        std::lock_guard<std::mutex> lock(mutex);

        std::thread::id this_thread_id = std::this_thread::get_id();
        auto transaction_itr = transactions.find(this_thread_id);
        if (transaction_itr != transactions.end()) {
            for (const auto &[key, value]: transaction_itr->second.changes) {
                if (value.has_value()) {
                    data[key] = value.value();
                    cache.put(key, value.value());
                } else {
                    data.erase(key);
                    cache.remove(key);
                }
            }
            transactions.erase(transaction_itr);
            return true;
        }
        return false;
    }

    bool abort_transaction() override {
        std::lock_guard<std::mutex> lock(mutex);

        std::thread::id this_thread_id = std::this_thread::get_id();

        if (!transactions.contains(this_thread_id)) {
            return false; // we can not abort transaction if it has not been initialized
        }

        transactions.erase(this_thread_id);
        return true;
    }

    std::string get(const std::string &key) const override {
        std::lock_guard<std::mutex> lock(mutex);

        // Trying to get information if there is active transaction and value have been changed in cache_itr
        std::thread::id this_thread_id = std::this_thread::get_id();
        auto transaction_itr = transactions.find(this_thread_id);
        if (transaction_itr != transactions.end()) {
            auto operation_it = transaction_itr->second.changes.find(key);
            if (operation_it != transaction_itr->second.changes.end()) {
                return operation_it->second.value_or(
                        ""); // returns inner transaction modified value or empty string if data has been deleted
            }
        }

        // Trying to get value from cash
        if (cache.is_cached(key)) {
            return cache.get(key);
        }

        // Trying to get value fromm db
        auto db_itr = data.find(key);
        if (db_itr != data.end()) {
            cache.put(key, db_itr->second);
            return db_itr->second;
        }

        return ""; // returns empty string if any data has not been found
    }

    std::string set(const std::string &key, const std::string &value) override {
        std::lock_guard<std::mutex> lock(mutex);

        std::thread::id this_thread_id = std::this_thread::get_id();
        auto trans_it = transactions.find(this_thread_id);
        if (trans_it != transactions.end()) {
            trans_it->second.changes[key] = value;
        } else {
            throw std::runtime_error(
                    "Setting data to database without active transaction"); // we cannot modify db without active transaction
        }

        return value;
    }

    std::string remove(const std::string &key) override {

        auto removed_data = get(key);

        std::lock_guard<std::mutex> lock(mutex);

        std::thread::id this_thread_id = std::this_thread::get_id();
        auto transaction_itr = transactions.find(this_thread_id);
        if (transaction_itr != transactions.end()) {
            transaction_itr->second.changes[key] = std::nullopt;
        } else {
            throw std::runtime_error(
                    "Removing from database without active transaction"); // we cannot modify db without active transaction
        }

        return removed_data; // since we assume that transaction has not been committed yet, this is legit code
    }

private:
    std::unordered_map<std::string, std::string> data;
    mutable T cache;
    mutable std::mutex mutex; // mutable because we want to lock does not affect const modifier for functions

    // Transaction struct is needed to represent current state of transaction and perform rollbacks from abortion
    struct Transaction {
        std::unordered_map<std::string, std::optional<std::string>> changes;
    };

    // Represents active transactions
    // It could be done with `thread_local`, but this approach potentially could give us ability to get active transactions if needed
    std::map<std::thread::id, Transaction> transactions;
};

#endif //TRANSACTIONAL_DATABASE_H
