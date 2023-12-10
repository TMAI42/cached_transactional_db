//
// Created by tmai42 on 12/8/23.
//

#ifndef TRANSACTIONAL_DATABASE_H
#define TRANSACTIONAL_DATABASE_H

#include "i_db.h"
#include "caches/lru_cache.h"
#include "caches/sized_cache_concept.h"

#include <optional>
#include <map>
#include <unordered_map>
#include <mutex>
#include <thread>

/**
 * class transactional_database
 *
 * This class implements a transaction-based database system, it
 *
 * It supports basic CRUD (Create, Read, Update, Delete) operations within the scope
 * of a transaction. Changes made in a transaction are only visible to that transaction until
 * they are committed. If a transaction is aborted, all traction data would be rolled back.
 *
 * @tparam T The type of cache used, defaulting to an LRU cache. The cache type must conform
 *           to the sized_cache_concept, which requires methods put, get, remove, is_cashed and
 *           constructor with size_t parameter.
 */
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

        return removed_data;
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
