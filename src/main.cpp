
#include "dbs/transactional_database.h"

#include <iostream>

int main() {
    transactional_database db(5);

    // Begin transaction
    db.begin_transaction();

    // Modify values in the transaction
    std::cout << "Setted key1: " << db.set("key1", "new_value1") << "\n";
    std::cout << "Setted key2: " << db.set("key2", "new_value2") << "\n";
    std::cout << "Removed key2 value: " << db.remove("key2") << "\n";

    // Commit the transaction
    db.commit_transaction();

    // Check values after the transaction
    std::cout << "key1: " << db.get("key1") << std::endl;  // Should print "key1: new_value1"
    std::cout << "key2: " << db.get("key2") << std::endl;  // Should print "key2: "

    return 0;
}

