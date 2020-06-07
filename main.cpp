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
#include "numeric_comparator.h"
#include "../interval-base-tree/src/tree.h"
#include "../interval-base-tree/src/leaftree.h"
#include "../interval-base-tree/src/config.h"

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
    return (i1->interval.left < i2->interval.left);
}

string getFileName(Tinterval & interval) {
    string folder_path = outputfolder + FLAGS_strategy;
    string file_name = folder_path + "/interval-";
    mkdir(folder_path.c_str(), 0755);
    file_name += to_string(interval.left) + "-" + to_string(interval.right) + ".txt";
    return file_name;
}


long original(vector <Tinterval> & queries, rocksdb::DB* db) {
    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    long sum = 0;
    auto st_1 = chrono::system_clock::now();
    for (int i = 0; i < queries.size(); i += 1) {
        Tinterval interval = queries[i];
        string start = to_string(interval.left);
        string limit = to_string(interval.right);
        if (FLAGS_write_disk) {
            string file_name = getFileName(interval);
            ofstream ofile(file_name);
            for (it->Seek(start);
                it->Valid() && stoi(it->key().ToString()) < interval.right;
                it->Next())
            {
                sum += stoi(it->value().ToString());
                ofile << it->key().ToString() << " - " << it->value().ToString() << "\n";
            }
            ofile.close();
        } else {
            for (it->Seek(start);
                it->Valid() && stoi(it->key().ToString()) < interval.right;
                it->Next())
            {
                sum += stoi(it->value().ToString());
            }
        }
    }
    auto et_1 = chrono::system_clock::now();
    delete it;
    chrono::duration<double> elapsed_seconds = et_1 - st_1;
    cout << "Time: " << elapsed_seconds.count() << endl;
    return sum;
}

long extra(vector <Tinterval> & queries, rocksdb::DB* db) {
    long sum = 0;
    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    Tree <ConfigExtra<T> > * tree = new Tree<ConfigExtra<T> >(FLAGS_leaf_size);

    double allocation_time = 0;
    double db_exec_time = 0;
    double extra_time = 0;
    double tree_time = 0;
    auto st_1 = chrono::system_clock::now();
    for (int i = 0; i < queries.size(); i += 1) {
        tree->insert_interval(queries[i]);
    }
    auto et_1 = chrono::system_clock::now();
    chrono::duration<double> elapsed_seconds = et_1 - st_1;
    tree_time = elapsed_seconds.count();

    st_1 = chrono::system_clock::now();

    LeafTree<T> leaftree;
    vector <Tinterval *> leafs;
    tree->getLeafs(leafs);
    random_shuffle(leafs.begin(), leafs.end());
    for (int i = 0; i < leafs.size(); i++) {
        leaftree.insert(*leafs[i]);
    }

    for (int i = 0; i < queries.size(); i++) {
        leaftree.assign(&queries[i]);
    }
    vector <LeafNode<T> *> nodes = leaftree.nodes();
    sort(nodes.begin(), nodes.end(), compareInterval);
    et_1 = chrono::system_clock::now();
    elapsed_seconds = et_1 - st_1;
    extra_time = elapsed_seconds.count();

    for (int i = 0; i < nodes.size(); i += 1) {
        auto st_2 = std::chrono::system_clock::now();
        Interval<T> leaf = nodes[i]->interval;
        string start = to_string(leaf.left);
        string limit = to_string(leaf.right);
        string ** temp = new string * [leaf.distance()];

        for (T i = 0; i < (leaf.right - leaf.left); i += 1) {
            temp[i] = NULL;
        }
        for (it->Seek(start);
            it->Valid() && stoi(it->key().ToString()) < leaf.right;
            it->Next()
        ) {
            T key = stoi(it->key().ToString());
            temp[key - leaf.left] = new string(it->value().ToString());
        }
        auto et_2 = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_timer = et_2 - st_2;
        allocation_time += elapsed_timer.count();

        st_2 = std::chrono::system_clock::now();
        for(int j = 0; j < nodes[i]->hashmap.size(); j++) {
            Tinterval * query = nodes[i]->hashmap[j].first;
            Tinterval interval = nodes[i]->hashmap[j].second;
            if (FLAGS_write_disk) {
                string file_name = getFileName(*query);
                ofstream ofile(file_name, std::ios_base::app);
                for (int i = interval.left; i < interval.right && temp[i - leaf.left] != NULL; i += 1) {
                    ofile << i << " - " << (*temp[i - leaf.left]) << "\n";
                    sum += stoi(*temp[i - leaf.left]);
                }
                ofile.close();
            } else {
                for (int i = interval.left; i < interval.right && temp[i - leaf.left] != NULL; i += 1) {
                    sum += stoi(*temp[i - leaf.left]);
                }
            }
        }
        et_2 = std::chrono::system_clock::now();
        elapsed_timer = et_2 - st_2;
        db_exec_time += elapsed_timer.count();
    }
    delete it;

    T * leafs_values = tree->getLeafsData();

    Result result(
        FLAGS_iter,
        FLAGS_distribution, queries.size(), FLAGS_strategy,
        FLAGS_key_domain_size, FLAGS_leaf_size, FLAGS_range_size,
        leaf_values,
        tree_time, tree->update_time, extra_time,
        db_exec_time, allocation_time,
        allocation_time + db_exec_time + tree_time
    );
    result.printCSV();

    return sum;
}


long lazy(vector <Tinterval> & queries, rocksdb::DB* db) {
    long sum = 0;
    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    Tree <ConfigLazy<T> > * tree = new Tree<ConfigLazy<T> >(FLAGS_leaf_size);

    double allocation_time = 0;
    double db_exec_time = 0;
    auto st_1 = chrono::system_clock::now();
    for (int i = 0; i < queries.size(); i += 1) {
        tree->insert_interval(queries[i]);
    }
    auto et_1 = chrono::system_clock::now();
    chrono::duration<double> elapsed_seconds = et_1 - st_1;
    double tree_time = elapsed_seconds.count();

    for (int i = 0; i < tree->root->leafs->size(); i += 1) {
        auto st_2 = std::chrono::system_clock::now();
        Tinterval leaf = tree->root->leafs->at(i)->interval;
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
        auto et_2 = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_timer = et_2 - st_2;
        allocation_time += elapsed_timer.count();

        st_2 = std::chrono::system_clock::now();

        for (vector<Tinterval *>::iterator it = tree->root->leafs->at(i)->queries->begin(); it != tree->root->leafs->at(i)->queries->end(); it++) {
            Tinterval * query = *it;
            if (query->right < leaf.left || query->left > leaf.right) {
                continue;
            }
            T begin = query->left - leaf.left >= 0 ? query->left - leaf.left : 0;
            T end = query->right - leaf.right >= 0 ? leaf.right - leaf.left : query->right - leaf.left;
            if (FLAGS_write_disk) {
                string file_name = getFileName(*query);
                ofstream ofile(file_name, std::ios_base::app);
                for (int i = begin; i < end; i += 1) {
                    ofile << i + leaf.left << " - " << (*temp[i]) << "\n";
                    sum += stoi(*temp[i]);
                }
                ofile.close();
            } else {
                for (int i = begin; i < end; i += 1) {
                    sum += stoi(*temp[i]);
                }
            }
        }
        et_2 = std::chrono::system_clock::now();
        elapsed_timer = et_2 - st_2;
        db_exec_time += elapsed_timer.count();
    }
    delete it;

    T * leafs_values = tree->getLeafsData();

    Result result(
        FLAGS_iter,
        FLAGS_distribution, queries.size(), FLAGS_strategy,
        FLAGS_key_domain_size, FLAGS_leaf_size, FLAGS_range_size,
        leaf_values,
        tree_time, tree->update_time, 0,
        db_exec_time, allocation_time,
        allocation_time + db_exec_time + tree_time
    );
    result.printCSV();

    return sum;
}


long eager(vector <Tinterval> & queries, rocksdb::DB* db) {
    long sum = 0;
    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    Tree <ConfigEager<T> > * tree = new Tree<ConfigEager<T> >(FLAGS_leaf_size);

    double allocation_time = 0;
    double db_exec_time = 0;
    auto st_1 = chrono::system_clock::now();
    for (int i = 0; i < queries.size(); i += 1) {
        tree->insert_interval(queries[i]);
    }
    auto et_1 = chrono::system_clock::now();
    chrono::duration<double> elapsed_seconds = et_1 - st_1;
    double tree_time = elapsed_seconds.count();

    for (int i = 0; i < tree->root->leafs->size(); i += 1) {
        auto st_2 = std::chrono::system_clock::now();
        Tinterval leaf = tree->root->leafs->at(i)->interval;
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
        auto et_2 = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_timer = et_2 - st_2;
        allocation_time += elapsed_timer.count();

        st_2 = std::chrono::system_clock::now();

        for (vector<pair<Tinterval *, Tinterval>>::iterator it = tree->root->leafs->at(i)->queries_map->begin(); it != tree->root->leafs->at(i)->queries_map->end(); it++) {
            Tinterval * query = it->first;
            Tinterval limits = it->second;

            if (FLAGS_write_disk) {
                string file_name = getFileName(*query);
                ofstream ofile(file_name, std::ios_base::app);
                for (T i = limits.left - leaf.left; i < (limits.right - leaf.left); i += 1) {
                    sum += stoi(*temp[i]);
                    ofile << i + leaf.left << " - " << (*temp[i]) << "\n";
                }
                ofile.close();
            } else {
                for (T i = limits.left - leaf.left; i < (limits.right - leaf.left); i += 1) {
                    sum += stoi(*temp[i]);
                }
            }

        }
        et_2 = std::chrono::system_clock::now();
        elapsed_timer = et_2 - st_2;
        db_exec_time += elapsed_timer.count();
    }
    delete it;

    T * leafs_values = tree->getLeafsData();

    Result result(
        FLAGS_iter,
        FLAGS_distribution, FLAGS_queries, FLAGS_strategy,
        FLAGS_key_domain_size, FLAGS_leaf_size, FLAGS_range_size,
        leaf_values,
        tree_time, tree->update_time, 0,
        db_exec_time, allocation_time,
        allocation_time + db_exec_time + tree_time
    );
    result.printCSV();

    return sum;
}


int main(int argc, char** argv) {
    srand (seed);

    gflags::SetUsageMessage("Reads Lazy");
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
        sum = extra(queries, db);
    } else if (FLAGS_strategy == "eager") {
        sum = eager(queries, db);
    }

    delete db;

    gflags::ShutDownCommandLineFlags();

    return 0;
}
