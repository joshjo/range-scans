
#include <assert.h>
#include <iostream>
#include <fstream>
#include <typeinfo>
#include <chrono>
#include <ctime>
#include <gflags/gflags.h>

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "numeric_comparator.h"
#include "../interval-base-tree/src/tree.h"
#include "../interval-base-tree/src/leaftree.h"
#include "../interval-base-tree/src/config.h"

typedef long T;
typedef Interval<T> Tinterval;

DEFINE_int64(key_domain_size, 1000000, "Key domain size");
DEFINE_int64(leaf_size, 100000, "Leaf size");
DEFINE_int64(query_number, 100, "Number of queries");
DEFINE_int64(range_size, 100000, "Range of queries");
DEFINE_string(strategy, "eager", "Strategy");


using namespace std;


vector<Tinterval> create_queries() {
    vector<Tinterval> result;
    T max_random = FLAGS_key_domain_size - FLAGS_leaf_size;

    for (int i = 0; i < FLAGS_query_number; i += 1) {
        T rnd = rand() % max_random;
        result.push_back(Tinterval(rnd, rnd + FLAGS_leaf_size));
    }
}


void lazy() {
    Tree <ConfigBase<T> > * tree;
    tree = new Tree<ConfigBase<T> >(FLAGS_leaf_size);
    // for (int i = 0; i < intervals.size(); i += 1) {
    //     tree.insert_interval(intervals[i], false);
    // }

    for(int i = 0; i < 5; i += 1) {
        cout << "i: " << rand() % 1000 << endl;
    }
}


int main(int argc, char** argv) {
    srand (100);

    gflags::SetUsageMessage("Reads Lazy");
    gflags::SetVersionString("1.0.0");
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    lazy();

    gflags::ShutDownCommandLineFlags();

    return 0;
}
