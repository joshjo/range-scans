#include <stdio.h>
#include <stdlib.h>

#include <assert.h>
#include <iostream>
#include <fstream>
#include <typeinfo>
#include <chrono>
#include <ctime>
#include <random>
#include <gflags/gflags.h>
#include <bits/stdc++.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "../interval-base-tree/src/tree.h"
#include "../interval-base-tree/src/leaftree.h"
#include "../interval-base-tree/src/config.h"

#include "numeric_comparator.h"

#include "utils.h"


using namespace std;

DEFINE_int64(key_domain_size, 1000000, "Key domain size");
DEFINE_int64(queries, 100, "Number of queries");
DEFINE_int64(range_size, 100000, "Range of queries");
DEFINE_string(query_type, "qat", "query type");


int main(int argc, char** argv) {
    gflags::SetUsageMessage("Reads Lazy");
    gflags::SetVersionString("1.0.0");
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    vector<Tinterval> queries = create_queries(
        FLAGS_queries, FLAGS_key_domain_size, FLAGS_range_size);


    rocksdb::DB* db;
    rocksdb::Options options;
    options.create_if_missing = true;
    NumericComparator cmp;
    options.comparator = &cmp;

    rocksdb::Status status = rocksdb::DB::Open(options, "/tmp/tree", &db);
    assert(status.ok());

    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());

    auto et_1 = chrono::system_clock::now();
    for (int i = 0; i < queries.size(); i++) {
        Tinterval interval = queries[i];

        if (FLAGS_query_type == "qat") {
            string value;
            for (int i = interval.left; i < interval.right; i += 1) {
                string key = to_string(i);
                db->Get(rocksdb::ReadOptions(), key, &value);
            }
        } else {
            string start = to_string(interval.left);
            string limit = to_string(interval.right);
            for (it->Seek(start);
                it->Valid() && stoi(it->key().ToString()) < interval.right;
                it->Next()) {
                    // ofile << it->key().ToString() << " - " << it->value().ToString() << "\n";
            }
        }



    }
    auto et_2 = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_timer = et_2 - et_1;
    delete it;
    delete db;

    cout << elapsed_timer.count() << endl;

    gflags::ShutDownCommandLineFlags();

    return 0;

}
