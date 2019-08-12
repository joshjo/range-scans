#include <assert.h>
#include <iostream>
#include "rocksdb/db.h"
#include "rocksdb/options.h"


int main() {
    rocksdb::DB* db;
    rocksdb::Options options;
    options.create_if_missing = true;
    rocksdb::Status status = rocksdb::DB::Open(options, "/tmp/rangescans", &db);

    if (status.ok()) {
        // for (long i = 0; i < 100000; i += 1) {
        //     std::string key = std::to_string(i);
        //     std::string value = std::to_string(i * 2);
        //     db->Put(rocksdb::WriteOptions(), key, value);
        // }

        std::string start = "100";
        std::string limit = "10000";
        int count = 0;
        rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
        for (it->Seek(start);
            it->Valid() && it->key().ToString() < limit;
            it->Next(), count += 1) {
                std::cout << it->key().ToString() << ": " << it->value().ToString() << std::endl;
            if (count > 100) {
                break;
            }
        }
    } else {
        printf("Error on opening file");
    }
    // assert(status.ok());
}
