#include <assert.h>
#include <iostream>
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "numeric_comparator.h"
#include "src/tree.h"
#include <chrono>
#include <ctime>


using namespace std;


int main() {
    srand (time(NULL));
    int S = 1000;
    int R = 150;

    vector <Interval <int> > intervals;
    Tree <int> tree;
    rocksdb::DB* db;
    rocksdb::Options options;
    options.create_if_missing = true;
    NumericComparator cmp;
    options.comparator = &cmp;

    rocksdb::Status status = rocksdb::DB::Open(options, "/tmp/tree", &db);
    assert(status.ok());
    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());

    for (int i = 0; i < 5; i += 1) {
        int S1 = rand() % S;
        int R1 = rand() % R;
        Interval<int> I(S1, S1 + R1);
        intervals.push_back(I);
        tree.insert_interval(I);
    }
    // auto start_time = std::chrono::system_clock::now();
    // for (auto & interval: intervals) {
    //     std::string start = to_string(interval.left);
    //     std::string limit = to_string(interval.right);

    //     for (it->Seek(start);
    //         it->Valid() && it->key().ToString() < limit;
    //         it->Next()) {
    //             // std::cout << it->key().ToString() << ": " << it->value().ToString() << std::endl;
    //     }
    //     // cout << "--> " << start << " * " << limit << endl;
    // }
    // auto end_time = std::chrono::system_clock::now();
    // std::chrono::duration<double> elapsed_seconds = end_time - start_time;
    // cout << "elapsed time: " << elapsed_seconds.count() << "s\n";

    auto start_time = std::chrono::system_clock::now();

    for (auto & x: intervals) {
        cout << "x: " << x << endl;

    }
    // tree.getLeafs();
    // cout << endl;
    auto end_time = std::chrono::system_clock::now();

    delete it;

    //     std::string key = std::to_string(it);
    //     std::string value = std::to_string(i * 2);
    //     db->Put(rocksdb::WriteOptions(), key, value);
    // }

    delete db;
}
