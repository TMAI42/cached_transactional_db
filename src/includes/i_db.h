//
// Created by tmai42 on 12/8/23.
//

#ifndef DB_H
#define DB_H

#include <string>

/*
 * struct i_db
 *
 * Represents simple interface for our db, and assumed as constant, so we have not provided any changes to it
 *
 * Note: this interface does not provide us any instruments to differentiate transactions between
 * each other in one thread, so we assume that that is only one unique transaction in each thread
 *
 * */
struct i_db {
    virtual bool begin_transaction() = 0;
    virtual bool commit_transaction() = 0;
    virtual bool abort_transaction() = 0;
    [[nodiscard]] virtual std::string get(const std::string& key) const = 0;
    virtual std::string set(const std::string& key, const std::string& data) = 0;
    virtual std::string remove(const std::string& key) = 0;

    virtual ~i_db() = default;
};

#endif //DB_H
