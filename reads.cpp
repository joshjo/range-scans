
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
#include "../interval-base-tree/src/config.h"

typedef Interval<int> Tinterval;


using namespace std;

vector <Tinterval> getVectorFromFile(int maxSize = 0, int range_size = 10) {
    vector <Tinterval> result;
    string filename = "data/range-" + to_string(range_size) + ".txt";
    ifstream infile(filename);
    int a, b;
    int i = 0;

    while (infile >> a >> b && i < maxSize) {
        Tinterval I(a, b);
        result.push_back(I);
        i += 1;
    }

    return result;
}


int main(int argc, char** argv) {
    srand (time(NULL));
    int a_size = 1;

    // int key_domain_size = 1000000;
    // int leaf_size = 10;
    // int R = leaf_size * 1.5;
    int query_number, key_domain_size, leaf_size, range_size, R;

    if (argc != 5) {
        cout << "Please enter a query size, domain, and leaf size: ./reads 100 1000 100" << endl;
        return 0;
    }

    sscanf(argv[1], "%d", &key_domain_size);
    sscanf(argv[2], "%d", &leaf_size);
    sscanf(argv[3], "%d", &query_number);
    sscanf(argv[4], "%d", &range_size);
    R = leaf_size * 1.5;

    double a_raw_time = 0;
    double a_tree_time = 0;
    double a_second_tree_time = 0;
    double a_query_time = 0;
    double a_results_time = 0;
    double a_db_time = 0;
    int range_avg = 0;
    int a_leafs_size = 0;
    int a_extra_insertion = 0;

    for (int k = 0; k < a_size; k++) {

        rocksdb::DB* db;
        rocksdb::Options options;
        options.create_if_missing = true;
        NumericComparator cmp;
        options.comparator = &cmp;

        rocksdb::Status status = rocksdb::DB::Open(options, "/tmp/tree", &db);
        assert(status.ok());
        rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
        int local_range_avg = 0;

        vector <Tinterval > queries = getVectorFromFile(query_number, range_size);

        // vector <Tinterval > queries;
        // for (int i = 0; i < (query_number); i += 1) {
        //     int S1 = rand() % key_domain_size;
        //     // int R1 = rand() % R;
        //     int R1 = leaf_size;
        //     Tinterval I(S1, S1 + leaf_size);
        //     local_range_avg += R1;
        //     queries.push_back(I);
        // }

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

                    // ofile << it->key().ToString() << " - " << it->value().ToString() << "\n";
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
        Tree <TraitsBase> tree(leaf_size);
        for (int i = 0; i < queries.size(); i += 1) {
            tree.insert_interval(queries[i]);
        }
        end_time = std::chrono::system_clock::now();

        elapsed_seconds = end_time - start_time;
        a_tree_time += elapsed_seconds.count();

        start_time = std::chrono::system_clock::now();
        LeafTree<int> leaftree;
        vector <Tinterval> leafs;

        tree.getLeafs(leafs);

        cout << leafs.size() << " - " << tree.root->leafs.size() << endl;

        for (int i = 0; i < leafs.size(); i++) {
            leaftree.insert(leafs[i]);
        }

        for (int i = 0; i < queries.size(); i++) {
            leaftree.assign(&queries[i]);
        }
        end_time = std::chrono::system_clock::now();

        elapsed_seconds = end_time - start_time;
        a_second_tree_time += elapsed_seconds.count();

        start_time = std::chrono::system_clock::now();

        vector <LeafNode<int> *> nodes = leaftree.nodes();

        double map_results_time = 0;
        double db_exec_time = 0;

        for (int i = 0; i < nodes.size(); i++) {
            auto timer_start = std::chrono::system_clock::now();
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

            auto timer_end = std::chrono::system_clock::now();

            std::chrono::duration<double> elapsed_timer = timer_end - timer_start;
            db_exec_time += elapsed_timer.count();

            timer_start = std::chrono::system_clock::now();

            for(int j = 0; j < nodes[i]->hashmap.size(); j++) {
                Interval<int> * query = nodes[i]->hashmap[j].first;
                Interval<int> interval = nodes[i]->hashmap[j].second;
                // string file_name = "results/indexed/interval-";
                // file_name += to_string(query->left) + " - " + to_string(query->right) + ".txt";
                // ofstream ofile(file_name, std::ios_base::app);
                for (int i = interval.left; i < interval.right && temp[i - leaf.left] != NULL; i += 1) {
                //     ofile << i << " - " << (*temp[i - leaf.left]) << "\n";
                }
                // ofile.close();
            }

            timer_end = std::chrono::system_clock::now();
            elapsed_timer = timer_end - timer_start;
            map_results_time += elapsed_timer.count();
        }

        // cout << endl << endl;

        // cout << tree.graphviz() << endl;

        a_db_time += db_exec_time;
        a_results_time += map_results_time;

        end_time = std::chrono::system_clock::now();

        elapsed_seconds = end_time - start_time;
        a_query_time += elapsed_seconds.count();

        a_leafs_size += nodes.size();
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

    a_db_time = a_db_time / a_size;
    a_results_time = a_results_time / a_size;
    range_avg = range_avg / a_size;


    double a_total_time = a_second_tree_time + a_tree_time + a_query_time;

    // cout << "Leafs size: " << a_leafs_size << endl;
    // cout << "Tiempo QAT: " << a_raw_time << endl;
    // cout << "Tree time : " << a_tree_time << endl;
    // cout << "2Tree Time: " << a_second_tree_time << endl;
    // cout << "Query time: " << a_query_time << endl;
    // cout << "dbexe time: " << a_db_time << endl;
    // cout << "mresu time: " << a_results_time << endl;
    // cout << "Tiempo tot: " << a_total_time << endl;
    // cout << "Interv avg: " << range_avg << endl;

    cout << a_leafs_size << ",";
    cout << a_raw_time << ",";
    cout << a_tree_time << ",";
    cout << a_second_tree_time << ",";
    cout << a_db_time << ",";
    cout << a_results_time << ",";
    cout << endl;
}
