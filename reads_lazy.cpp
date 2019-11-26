
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

typedef int T;
typedef Interval<T> Tinterval;

DEFINE_int64(key_domain_size, 1000000, "Key domain size");
DEFINE_int64(leaf_size, 100000, "Leaf size");
DEFINE_int64(query_number, 100, "Number of queries");
DEFINE_int64(range_size, 100000, "Range of queries");


// class TraitsBase {
//     public:
//         typedef int T;
//         typedef Node<T> TNode;
// };


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
    gflags::SetUsageMessage("Reads Lazy");
    gflags::SetVersionString("1.0.0");
    gflags::ParseCommandLineFlags(&argc, &argv, true);


    // cout << "big_menu" << FLAGS_key_domain_size << endl;
    // srand (time(NULL));
    int a_size = 1;

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

        vector <Tinterval > queries = getVectorFromFile(FLAGS_query_number, FLAGS_range_size);

        // vector <Tinterval > queries;
        // for (int i = 0; i < (query_number); i += 1) {
        //     int S1 = rand() % key_domain_size;
        //     // int R1 = rand() % R;
        //     int R1 = leaf_size;
        //     Tinterval I(S1, S1 + leaf_size);
        //     local_range_avg += R1;
        //     queries.push_back(I);
        // }

        range_avg += local_range_avg / FLAGS_query_number;

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
        Tree <TraitsBase> tree(FLAGS_leaf_size);
        for (int i = 0; i < queries.size(); i += 1) {
            tree.insert_interval(queries[i]);
        }
        end_time = std::chrono::system_clock::now();

        elapsed_seconds = end_time - start_time;
        a_tree_time += elapsed_seconds.count();

        start_time = std::chrono::system_clock::now();
        // LeafTree<int> leaftree;

        // vector <Tinterval> leafs;

        // tree.getLeafs(leafs);

        // cout << leafs.size() << " - " << tree.root->leafs.size() << endl;

        // for (int i = 0; i < leafs.size(); i++) {
        //     leaftree.insert(leafs[i]);
        // }

        // for (int i = 0; i < queries.size(); i++) {
        //     leaftree.assign(&queries[i]);
        // }
        end_time = std::chrono::system_clock::now();

        elapsed_seconds = end_time - start_time;
        a_second_tree_time += elapsed_seconds.count();

        start_time = std::chrono::system_clock::now();

        // vector <LeafNode<int> *> nodes = leaftree.nodes();


        double map_results_time = 0;
        double db_exec_time = 0;

        for (int i = 0; i < tree.root->leafs.size(); i += 1) {

            auto timer_start = std::chrono::system_clock::now();
            Tinterval leaf = tree.root->leafs[i]->interval;
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

            for (set<Tinterval *>::iterator it = tree.root->leafs[i]->queries.begin(); it != tree.root->leafs[i]->queries.end(); it++) {
                Tinterval * query = *it;

                if (query->right < leaf.left || query->left > leaf.right) {
                    // cout << "break: " << query->left << " - " << query->right << " | ";
                    // cout << leaf.left << " - " << leaf.right << endl;
                    continue;
                }
                T begin = query->left - leaf.left >= 0 ? query->left - leaf.left : 0;
                T end = query->right - leaf.right >= 0 ? leaf.right - leaf.left : query->right - leaf.left;

                for (int i = begin; i < end; i += 1) {}
            }

            timer_end = std::chrono::system_clock::now();
            elapsed_timer = timer_end - timer_start;
            map_results_time += elapsed_timer.count();
        }

        a_db_time += db_exec_time;
        a_results_time += map_results_time;

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

    a_db_time = a_db_time / a_size;
    a_results_time = a_results_time / a_size;
    range_avg = range_avg / a_size;


    double a_total_time = a_tree_time + a_query_time + a_db_time;

    // cout << "Leafs size: " << a_leafs_size << endl;
    // cout << "Tiempo QAT: " << a_raw_time << endl;
    // cout << "Tree time : " << a_tree_time << endl;
    // cout << "2Tree Time: " << a_second_tree_time << endl;
    // cout << "Query time: " << a_query_time << endl;
    // cout << "dbexe time: " << a_db_time << endl;
    // cout << "mresu time: " << a_results_time << endl;
    // cout << "Tiempo tot: " << a_total_time << endl;
    // cout << "Interv avg: " << range_avg << endl;

    std::cout << a_leafs_size << ",";
    std::cout << a_raw_time << ",";
    // std::cout << a_tree_time << ",";
    // std::cout << a_query_time << ",";
    // std::cout << a_db_time << ",";
    std::cout << a_query_time + a_tree_time << endl;
    // std::cout << endl;
    gflags::ShutDownCommandLineFlags();
    return 0;
}
