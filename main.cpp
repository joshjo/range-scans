#include <assert.h>
#include <iostream>
#include "rocksdb/db.h"
#include "rocksdb/options.h"

using namespace std;


class NumericComparator : public rocksdb::Comparator {
public:
    // Three-way comparison function:
    //   if a < b: negative result
    //   if a > b: positive result
    //   else: zero result
    int Compare(const rocksdb::Slice& a, const rocksdb::Slice& b) const {
        int a1, b1;
        ParseKey(a, &a1);
        ParseKey(b, &b1);
        if (a1 < b1) return -1;
        if (a1 > b1) return +1;

        return 0;
    }

    void ParseKey(const rocksdb::Slice& a, int* a1) const {
    	std::string parts = a.ToString();
    	*a1 = atoi(parts.c_str());
    }

    // Ignore the following methods for now:
    const char* Name() const { return "NumericComparator"; }
    void FindShortestSeparator(std::string*, const rocksdb::Slice&) const { }
    void FindShortSuccessor(std::string*) const { }
};


int main() {
    rocksdb::DB* db;
    rocksdb::Options options;
    options.create_if_missing = true;
    NumericComparator cmp;
    options.comparator = &cmp;

    // rocksdb::Status status = rocksdb::DB::Open(options, "/tmp/rangescans", &db);

    //  // open a database with custom comparator
    rocksdb::Status status = rocksdb::DB::Open(options, "/tmp/testdb", &db);
    assert(status.ok());

    // populate the database
    rocksdb::Slice key1 = "13";
    std::string val1 = "one";
    // rocksdb::Slice key2 = "2:3";
    // rocksdb::Slice key3 = "2:1";
    // std::string val2 = "two";
    // std::string val3 = "three";
    // db->Put(rocksdb::WriteOptions(), key1, val1);
    // for (long i = 0; i < 100000; i += 1) {
    //     std::string key = std::to_string(i);
    //     std::string value = std::to_string(i * 2);
    //     db->Put(rocksdb::WriteOptions(), key, value);
    // }

    //iterate the database
    // rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    // for (it->SeekToFirst(); it->Valid(); it->Next()) {
    //     cout << it->key().ToString() << ": " << it->value().ToString() << endl;
    // }

    std::string start = "100";
    std::string limit = "200";
    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    for (it->Seek(start);
        it->Valid() && it->key().ToString() < limit;
        it->Next()) {
            std::cout << it->key().ToString() << ": " << it->value().ToString() << std::endl;
    }
    // delete it;

    // close the database
    delete db;
}
