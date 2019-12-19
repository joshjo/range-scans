#include <assert.h>
#include <iostream>
#include <fstream>
#include <typeinfo>
#include <chrono>
#include <ctime>

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "numeric_comparator.h"
#include "../interval-base-tree/src/tree.h"
#include "../interval-base-tree/src/leaftree.h"


using namespace std;


int main(int argc, char** argv) {
    // if (argc != 3) {
    //     cout << "Please enter a range: ./simple 100 1000" << endl;
    //     return 0;
    // }

    // int start, limit;

    // sscanf(argv[1], "%d", &start);
    // sscanf(argv[2], "%d", &limit);

    // string sstart = to_string(start);
    // string slimit = to_string(limit);

    int key_domain_size = 1000000;

    rocksdb::DB* db;
    rocksdb::Options options;
    options.create_if_missing = true;
    NumericComparator cmp;
    options.comparator = &cmp;

    rocksdb::Status status = rocksdb::DB::Open(options, "/tmp/tree", &db);
    assert(status.ok());
    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    int local_range_avg = 0;

    for (int i = 0; i < 100; i += 1) {
        int S1 = rand() % key_domain_size;
        string sstart = to_string(S1);
        int limit = S1 + 1;
        for (it->Seek(sstart);
            it->Valid() && stoi(it->key().ToString()) < limit;
            it->Next()) {
                // cout << it->key().ToString() << " - " << it->value().ToString() << "\n";
        }
    }

    // cout << ("999" < "1000") << endl;

    delete it;
    delete db;

    return 0;
}
