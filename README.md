# Simple Cached Transaction Database 

## Overview

This is simple transaction database, implemented based on given interface 
```cpp
struct i_db {
    virtual bool begin_transaction() = 0;
    virtual bool commit_transaction() = 0;
    virtual bool abort_transaction() = 0;
    [[nodiscard]] virtual std::string get(const std::string& key) const = 0;
    virtual std::string set(const std::string& key, const std::string& data) = 0;
    virtual std::string remove(const std::string& key) = 0;

    virtual ~i_db() = default;
};
```

### Prerequisites
- C++23 compatible compiler
- CMake (version 3.26 or higher)

## Quick Start

### Build
```bash
mkdir build
cd build
cmake ..
make
```

### Run

```bash
./cached_transactional_db
```

## Core features

 - **DB** implements *lru cache* to speedup data lookups, for recent added data
 - **Rollback implementation**. Uncommitted changes would be discarded after `transaction_abort()` call
 - **Thread safety**. All classes are thread safe even if they are used separately from each other
 - **Unique transaction** is implemented for each thread

## Future improvements

- **Error handling**. For now there is no universal wap to handle errors
- **Add tests module**. It would be great to have tests using [GTest](https://github.com/google/googletest)
- **Singleton pattern for db**. Maybe would be a great idea to make db as a singleton in the future, but for now there is no 
objective reasons for it


## Resources 

- [Cache replacement policies](https://en.wikipedia.org/wiki/Cache_replacement_policies)
- [Commit and rollback](https://www.ibm.com/docs/en/cics-ts/5.4?topic=processing-commit-rollback)
- [ACID properties of transactions](https://www.ibm.com/docs/en/cics-ts/5.4?topic=processing-acid-properties-transactions)
- [An Introduction to Transactional Databases](https://www.mongodb.com/databases/types/transactional-databases)
