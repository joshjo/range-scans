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
    srand (time(NULL));
    typedef Interval<int> Tinterval;
    int a_size = 1;

    int key_domain_size = 1000000;
    int leaf_size = 100000;
    int R = 150000;
    int query_number;
    map <Tinterval, string> results;

    double a_raw_time = 0;
    double a_tree_time = 0;
    double a_second_tree_time = 0;
    double a_query_time = 0;
    int range_avg = 0;
    int a_leafs_size = 0;
    int a_extra_insertion = 0;

    if (argc != 2) {
        cout << "Please enter an initial size" << endl;
        return 1;
    }

    sscanf(argv[1], "%d", &query_number);

    for (int k = 0; k < a_size; k++) {
        vector <Tinterval > queries;

        rocksdb::DB* db;
        rocksdb::Options options;
        options.create_if_missing = true;
        NumericComparator cmp;
        options.comparator = &cmp;

        rocksdb::Status status = rocksdb::DB::Open(options, "/tmp/tree", &db);
        assert(status.ok());
        rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
        int local_range_avg = 0;

        for (int i = 0; i < (query_number); i += 1) {
            int S1 = rand() % key_domain_size;
            int R1 = rand() % R;
            Tinterval I(S1, S1 + R1);
            local_range_avg += R1;
            queries.push_back(I);
        }

        range_avg += local_range_avg / query_number;

        auto start_time = std::chrono::system_clock::now();

        for (int i = 0; i < queries.size(); i++) {
            Tinterval interval = queries[i];
            string start = to_string(interval.left);
            string limit = to_string(interval.right);
            string file_name = "results/original/interval-";
            file_name += to_string(interval.left) + " - " + to_string(interval.right) + ".txt";
            ofstream ofile(file_name);

            for (it->Seek(start);
                it->Valid() && stoi(it->key().ToString()) < interval.right;
                it->Next()) {

                    ofile << it->key().ToString() << " - " << it->value().ToString() << "\n";
            }
            ofile.close();
        }

        auto end_time = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end_time - start_time;
        a_raw_time += elapsed_seconds.count();
        // // cout << elapsed_seconds.count() << ",";
        // // int extra_insertions = 0;
        start_time = std::chrono::system_clock::now();
        // // double sum_total = 0;
        Tree <int> tree(leaf_size);
        for (int i = 0; i < queries.size(); i += 1) {
            tree.insert_interval(queries[i]);
        }
        end_time = std::chrono::system_clock::now();

        elapsed_seconds = end_time - start_time;
        a_tree_time += elapsed_seconds.count();

        start_time = std::chrono::system_clock::now();

        LeafTree<int> leaftree;

        for (int i = 0; i < tree.root->leafs.size(); i++) {
            leaftree.insert(tree.root->leafs[i]->interval);
        }

        for (int i = 0; i < queries.size(); i++) {
            leaftree.assign(&queries[i]);
        }

        elapsed_seconds = end_time - start_time;
        a_second_tree_time += elapsed_seconds.count();

        start_time = std::chrono::system_clock::now();

        vector <LeafNode<int> *> nodes = leaftree.nodes();

        for (int i = 0; i < nodes.size(); i++) {
            Interval<int> leaf = nodes[i]->interval;
            string start = to_string(leaf.left);
            string limit = to_string(leaf.right);
            string ** temp = new string * [leaf.distance()];

            for (int i = leaf.left; i < leaf.right; i += 1) {
                temp[i - leaf.left] = NULL;
            }
            for (it->Seek(start);
                it->Valid() && stoi(it->key().ToString()) < leaf.right;
                it->Next()
            ) {
                int key = stoi(it->key().ToString());
                temp[key - leaf.left] = new string(it->value().ToString());
            }
            for(int j = 0; j < nodes[i]->hashmap.size(); j++) {
                Interval<int> * query = nodes[i]->hashmap[j].first;
                Interval<int> interval = nodes[i]->hashmap[j].second;
                string file_name = "results/indexed/interval-";
                file_name += to_string(query->left) + " - " + to_string(query->right) + ".txt";
                ofstream ofile(file_name, std::ios_base::app);
                for(int i = interval.left; i < interval.right && temp[i - leaf.left] != NULL; i += 1) {
                    ofile << i << " - " << (*temp[i - leaf.left]) << "\n";
                }
                ofile.close();
            }
        }

        end_time = std::chrono::system_clock::now();

        elapsed_seconds = end_time - start_time;
        a_query_time += elapsed_seconds.count();

        a_leafs_size += tree.root->leafs.size();
        a_extra_insertion += tree.number_pending;

        delete it;

        delete db;
    }

    a_raw_time = a_raw_time / a_size;
    a_second_tree_time = a_second_tree_time / a_size;
    a_tree_time = a_tree_time / a_size;
    a_query_time = a_query_time / a_size;
    a_leafs_size = a_leafs_size / a_size;
    a_extra_insertion = a_extra_insertion / a_size;

    cout << a_raw_time << ",";
    cout << a_tree_time << ",";
    cout << a_second_tree_time << ",";
    cout << a_query_time << ",";
    cout << a_tree_time + a_query_time << ",";
    cout << a_leafs_size << ",";
    cout << a_extra_insertion << ",";
    cout << range_avg << ",";
    cout << endl;
}
