#include <assert.h>
#include <iostream>
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "numeric_comparator.h"
#include "../interval-base-tree/src/tree.h"
#include <chrono>
#include <ctime>


using namespace std;


int main(int argc, char** argv) {
    srand (time(NULL));
    typedef Interval<int> Tinterval;

    int key_domain_size = 10000000;
    int leaf_size = 1000000;
    int R = 1500000;
    int query_number;
    map <Tinterval, string> results;

    if (argc != 2) {
        cout << "Please enter an initial size" << endl;
        return 1;
    }

    sscanf(argv[1], "%d", &query_number);


    for (int k = 0; k < 1; k++) {
        vector <Tinterval > intervals;

        rocksdb::DB* db;
        rocksdb::Options options;
        options.create_if_missing = true;
        NumericComparator cmp;
        options.comparator = &cmp;

        rocksdb::Status status = rocksdb::DB::Open(options, "/tmp/tree", &db);
        assert(status.ok());
        rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());

        for (int i = 0; i < (query_number); i += 1) {
            int S1 = rand() % key_domain_size;
            int R1 = rand() % R;
            Tinterval I(S1, S1 + R1);
            intervals.push_back(I);
        }
        auto start_time = std::chrono::system_clock::now();
        for (auto & interval: intervals) {
            std::string start = to_string(interval.left);
            std::string limit = to_string(interval.right);

            for (it->Seek(start);
                it->Valid() && it->key().ToString() < limit;
                it->Next()) {
                    // std::cout << it->key().ToString() << ": " << it->value().ToString() << std::endl;
            }
            // cout << "--> " << start << " * " << limit << endl;
        }
        auto end_time = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end_time - start_time;
        // cout << elapsed_seconds.count() << ",";


        int extra_insertions = 0;
        start_time = std::chrono::system_clock::now();
        double sum_total = 0;
        Tree <int> tree(leaf_size);
        for (auto & interval: intervals) {
            extra_insertions += tree.insert_interval(interval);
        }
        end_time = std::chrono::system_clock::now();

        elapsed_seconds = end_time - start_time;
        sum_total += elapsed_seconds.count();
        // cout << elapsed_seconds.count() << ",";

        start_time = std::chrono::system_clock::now();
        vector<Tinterval > leafs;
        tree.getLeafs(leafs);

        for (int i = 0; i < leafs.size(); i++) {
            Tinterval interval = leafs[i];
            std::string start = to_string(interval.left);
            std::string limit = to_string(interval.right);

            for (it->Seek(start);
                it->Valid() && it->key().ToString() < limit;
                it->Next()) {
                    // results[interval] = it->value().ToString();
                    // std::cout << it->key().ToString() << ": " << it->value().ToString() << std::endl;
            }
            // cout << "--> " << start << " * " << limit << endl;

        }
        end_time = std::chrono::system_clock::now();

        elapsed_seconds = end_time - start_time;
        sum_total += elapsed_seconds.count();
        // cout << elapsed_seconds.count() << ",";
        // cout << sum_total << ",";
        // cout << leafs.size() << ",";
        // cout << extra_insertions << "\n";

        delete it;

        delete db;
    }

}
