#include <assert.h>
#include <iostream>
#include <fstream>
#include <typeinfo>
#include <chrono>
#include <ctime>
#include <random>
#include <gflags/gflags.h>
// #include <bits/stdc++.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "numeric_comparator.h"
#include "../interval-base-tree/src/newtree.h"

#include "result.h"
#include "utils.h"

typedef long T;
typedef Interval<T> Tinterval;

DEFINE_int64(key_domain_size, 1000000, "Key domain size");
DEFINE_int64(leaf_size, 100000, "Leaf size");
DEFINE_int64(queries, 100, "Number of queries");
DEFINE_int64(range_size, 100000, "Range of queries");
DEFINE_string(strategy, "raw", "Strategy");
DEFINE_string(distribution, "normal", "Random Distribution");
DEFINE_bool(write_disk, false, "Write output to disk");
DEFINE_int64(iter, 0, "Define the iteration number");
string outputfolder = "output/";

using namespace std;

int seed = 100;

bool compareInterval(LeafNode<T> * i1, LeafNode<T> * i2)
{
    return (i1->interval.min < i2->interval.min);
}

string getFileName(Tinterval & interval) {
    string folder_path = outputfolder + FLAGS_strategy;
    string file_name = folder_path + "/interval-";
    mkdir(folder_path.c_str(), 0755);
    file_name += to_string(interval.min) + "-" + to_string(interval.max) + ".txt";
    return file_name;
}


long original(vector <Tinterval> & queries, rocksdb::DB* db) {
    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    long sum = 0;
    auto st_1 = chrono::system_clock::now();
    for (int i = 0; i < queries.size(); i += 1) {
        Tinterval interval = queries[i];
        string start = to_string(interval.min);
        string limit = to_string(interval.max);
        if (FLAGS_write_disk) {
            string file_name = getFileName(interval);
            ofstream ofile(file_name);
            for (it->Seek(start);
                it->Valid() && stoi(it->key().ToString()) < interval.max;
                it->Next())
            {
                sum += stoi(it->value().ToString());
                ofile << it->key().ToString() << " - " << it->value().ToString() << "\n";
            }
            ofile.close();
        } else {
            for (it->Seek(start);
                it->Valid() && stoi(it->key().ToString()) < interval.max;
                it->Next())
            {
                sum += stoi(it->value().ToString());
            }
        }
    }
    auto et_1 = chrono::system_clock::now();
    delete it;
    chrono::duration<double> elapsed_seconds = et_1 - st_1;
    cout << FLAGS_iter << "," << FLAGS_distribution << "," << FLAGS_queries;
    cout << "," << elapsed_seconds.count() << endl;
    // cout << "Time: " << elapsed_seconds.count() << endl;
    return sum;
}


long additional(vector <Tinterval> & queries, rocksdb::DB* db) {
    long sum = 0;
    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());

    QMapExtra <Traits <T>> * qMap = new QMapExtra <Traits <T>>();
    Tree <Traits <T> > * tree = new Tree <Traits <T> >(FLAGS_leaf_size, qMap);

    double tree_total_time = 0;
    double mapping_time = 0;
    double post_filtering_time = 0;
    double db_exec_time = 0;

    auto st_1 = chrono::system_clock::now();
    for (int i = 0; i < queries.size(); i += 1) {
        tree->insert(queries[i]);
    }
    auto et_1 = chrono::system_clock::now();
    chrono::duration<double> elapsed_seconds = et_1 - st_1;
    tree_total_time = elapsed_seconds.count();

    st_1 = chrono::system_clock::now();
    LeafTree<Traits <T>> leaftree;
    vector<Node<T> *> leafs;
    tree->root->getLeafs(leafs);
    random_shuffle(leafs.begin(), leafs.end());

    for (int i = 0; i < leafs.size(); i++) {
        leaftree.insert(leafs[i]->interval);
    }

    for (int i = 0; i < queries.size(); i += 1) {
        leaftree.assign(&queries[i]);
    }

    vector <LeafNode<T> *> nodes = leaftree.nodes();
    sort(nodes.begin(), nodes.end(), compareInterval);

    et_1 = chrono::system_clock::now();
    elapsed_seconds = et_1 - st_1;
    mapping_time = elapsed_seconds.count();
    tree->qMap->indexed = leaftree.numIndexedQueries();

    for (auto itq = nodes.begin(); itq < nodes.end(); itq++) {
        auto st_2 = std::chrono::system_clock::now();
        Tinterval leaf = (*itq)->interval;
        string start = to_string(leaf.min);
        string limit = to_string(leaf.max);
        string ** temp = new string * [leaf.length()];

        for (int i = leaf.min; i < leaf.max; i += 1) {
            temp[i - leaf.min] = NULL;
        }
        for (it->Seek(start);
            it->Valid() && stoi(it->key().ToString()) < leaf.max;
            it->Next()
        ) {
            int key = stoi(it->key().ToString());
            temp[key - leaf.min] = new string(it->value().ToString());
        }
        auto et_2 = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_timer = et_2 - st_2;
        db_exec_time += elapsed_timer.count();

        st_2 = std::chrono::system_clock::now();
        for(size_t i = 0; i < (*itq)->hashmap.size(); i++) {
            Tinterval * query = (*itq)->hashmap[i].first;
            Tinterval limits = (*itq)->hashmap[i].second;

            if (FLAGS_write_disk) {
                // string file_name = getFileName(*query);
                // ofstream ofile(file_name, std::ios_base::app);
                // for (T i = limits.min - leaf.min; i < (limits.max - leaf.min); i += 1) {
                //     sum += stoi(*temp[i]);
                //     ofile << i + leaf.min << " - " << (*temp[i]) << "\n";
                // }
                // ofile.close();
            } else {
                for (T j = limits.min - leaf.min; j < (limits.max - leaf.min); j++) {
                    sum += stoi(*temp[j]);
                }
            }

        }
        et_2 = std::chrono::system_clock::now();
        elapsed_timer = et_2 - st_2;
        post_filtering_time += elapsed_timer.count();
    }
    delete it;

    Result result(
        FLAGS_iter,
        FLAGS_distribution, FLAGS_queries, FLAGS_strategy,
        FLAGS_key_domain_size, FLAGS_leaf_size, FLAGS_range_size,
        post_filtering_time, db_exec_time
    );
    result.printCSV(tree, tree_total_time, mapping_time);

    return sum;
}


long lazy(vector <Tinterval> & queries, rocksdb::DB* db) {
    long sum = 0;
    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());

    QMapLazy <Traits <T>> * qMap = new QMapLazy <Traits <T>>();
    Tree <Traits <T> > * tree = new Tree <Traits <T> >(FLAGS_leaf_size, qMap);

    double tree_total_time = 0;
    double post_filtering_time = 0;
    double db_exec_time = 0;

    auto st_1 = chrono::system_clock::now();
    for (int i = 0; i < queries.size(); i += 1) {
        tree->insert(queries[i]);
    }
    auto et_1 = chrono::system_clock::now();
    chrono::duration<double> elapsed_seconds = et_1 - st_1;
    tree_total_time = elapsed_seconds.count();

    for (auto itq = qMap->qMap.begin(); itq != qMap->qMap.end(); itq++) {
        auto st_2 = std::chrono::system_clock::now();
        Tinterval leaf = itq->first->interval;
        string start = to_string(leaf.min);
        string limit = to_string(leaf.max);
        string ** temp = new string * [leaf.length()];

        for (int i = leaf.min; i < leaf.max; i += 1) {
            temp[i - leaf.min] = NULL;
        }
        for (it->Seek(start);
            it->Valid() && stoi(it->key().ToString()) < leaf.max;
            it->Next()
        ) {
            int key = stoi(it->key().ToString());
            temp[key - leaf.min] = new string(it->value().ToString());
        }
        auto et_2 = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_timer = et_2 - st_2;
        db_exec_time += elapsed_timer.count();

        st_2 = std::chrono::system_clock::now();
        for (size_t i = 0; i < itq->second.size(); i++) {
            Tinterval * query = itq->second[i];
            Tinterval limits = itq->first->interval.intersection(*query);

            if (limits.length() == 0) {
                continue;
            }

            if (FLAGS_write_disk) {
                // string file_name = getFileName(*query);
                // ofstream ofile(file_name, std::ios_base::app);
                // for (T i = limits.min - leaf.min; i < (limits.max - leaf.min); i += 1) {
                //     sum += stoi(*temp[i]);
                //     ofile << i + leaf.min << " - " << (*temp[i]) << "\n";
                // }
                // ofile.close();
            } else {
                for (T j = limits.min - leaf.min; j < (limits.max - leaf.min); j++) {
                    sum += stoi(*temp[j]);
                }
            }

        }
        et_2 = std::chrono::system_clock::now();
        elapsed_timer = et_2 - st_2;
        post_filtering_time += elapsed_timer.count();
    }
    delete it;

    Result result(
        FLAGS_iter,
        FLAGS_distribution, FLAGS_queries, FLAGS_strategy,
        FLAGS_key_domain_size, FLAGS_leaf_size, FLAGS_range_size,
        post_filtering_time, db_exec_time
    );
    result.printCSV(tree, tree_total_time, 0);

    return sum;
}


long eager(vector <Tinterval> & queries, rocksdb::DB* db) {
    long sum = 0;
    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());

    QMapEager <Traits <T>> * qMap = new QMapEager <Traits <T>>();
    Tree <Traits <T> > * tree = new Tree <Traits <T> >(FLAGS_leaf_size, qMap);

    double tree_total_time = 0;
    double post_filtering_time = 0;
    double db_exec_time = 0;

    auto st_1 = chrono::system_clock::now();
    for (int i = 0; i < queries.size(); i += 1) {
        tree->insert(queries[i]);
    }
    auto et_1 = chrono::system_clock::now();
    chrono::duration<double> elapsed_seconds = et_1 - st_1;
    tree_total_time = elapsed_seconds.count();

    for (auto itq = qMap->qMap.begin(); itq != qMap->qMap.end(); itq++) {
        auto st_2 = std::chrono::system_clock::now();
        Tinterval leaf = itq->first->interval;
        string start = to_string(leaf.min);
        string limit = to_string(leaf.max);
        string ** temp = new string * [leaf.length()];

        for (int i = leaf.min; i < leaf.max; i += 1) {
            temp[i - leaf.min] = NULL;
        }
        for (it->Seek(start);
            it->Valid() && stoi(it->key().ToString()) < leaf.max;
            it->Next()
        ) {
            int key = stoi(it->key().ToString());
            temp[key - leaf.min] = new string(it->value().ToString());
        }
        auto et_2 = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_timer = et_2 - st_2;
        db_exec_time += elapsed_timer.count();

        st_2 = std::chrono::system_clock::now();
        for (size_t i = 0; i < itq->second.size(); i++) {
            Tinterval * query = itq->second[i].first;
            Tinterval limits = itq->second[i].second;

            if (FLAGS_write_disk) {
                // string file_name = getFileName(*query);
                // ofstream ofile(file_name, std::ios_base::app);
                // for (T i = limits.min - leaf.min; i < (limits.max - leaf.min); i += 1) {
                //     sum += stoi(*temp[i]);
                //     ofile << i + leaf.min << " - " << (*temp[i]) << "\n";
                // }
                // ofile.close();
            } else {
                for (T j = limits.min - leaf.min; j < (limits.max - leaf.min); j++) {
                    sum += stoi(*temp[j]);
                }
            }

        }
        et_2 = std::chrono::system_clock::now();
        elapsed_timer = et_2 - st_2;
        post_filtering_time += elapsed_timer.count();
    }
    delete it;

    Result result(
        FLAGS_iter,
        FLAGS_distribution, FLAGS_queries, FLAGS_strategy,
        FLAGS_key_domain_size, FLAGS_leaf_size, FLAGS_range_size,
        post_filtering_time, db_exec_time
    );
    result.printCSV(tree, tree_total_time, 0);

    return sum;
}


int main(int argc, char** argv) {
    srand (seed);

    gflags::SetUsageMessage("Main");
    gflags::SetVersionString("1.0.0");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    vector <Tinterval> queries;

    if (FLAGS_distribution == "zipf") {
        queries = create_queries_zipf(FLAGS_queries, FLAGS_key_domain_size, FLAGS_range_size);
    } else {
        queries = create_queries(FLAGS_queries, FLAGS_key_domain_size, FLAGS_range_size);
    }

    rocksdb::DB* db;
    rocksdb::Options options;
    options.create_if_missing = true;
    NumericComparator cmp;
    options.comparator = &cmp;

    rocksdb::Status status = rocksdb::DB::Open(options, "/tmp/tree", &db);
    assert(status.ok());

    long sum = 0;

    if (FLAGS_strategy == "lazy") {
        sum = lazy(queries, db);
    } else if (FLAGS_strategy == "original") {
        sum = original(queries, db);
    } else if (FLAGS_strategy == "additional") {
        sum = additional(queries, db);
    } else if (FLAGS_strategy == "eager") {
        sum = eager(queries, db);
    }

    delete db;

    gflags::ShutDownCommandLineFlags();

    return 0;
}
