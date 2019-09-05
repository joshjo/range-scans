#include <assert.h>
#include <iostream>
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "numeric_comparator.h"


using namespace std;


int main() {
    rocksdb::DB* db;
    rocksdb::Options options;
    options.create_if_missing = true;
    NumericComparator cmp;
    options.comparator = &cmp;

    rocksdb::Status status = rocksdb::DB::Open(options, "/tmp/tree", &db);
    assert(status.ok());

    for (long i = 0; i < 100000; i += 1) {
        std::string key = std::to_string(i);
        std::string value = std::to_string(i * 2);
        db->Put(rocksdb::WriteOptions(), key, value);
    }

    delete db;
}
