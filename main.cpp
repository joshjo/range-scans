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
#include "../interval-tree/src/newtree.h"

#include "result.h"
#include "utils.h"


DEFINE_int64(queries, 100, "Number of queries");
DEFINE_int64(key_domain_size, 1000000, "Key domain size");
DEFINE_string(leaf_size, "100000", "Leaf size");
DEFINE_int64(range_size, 100000, "Range of queries");
DEFINE_bool(random_range_size, false, "Use random range of queries");
DEFINE_int64(min_range_size, 100000, "Min random range of queries");
DEFINE_int64(max_range_size, 100000, "Max random range of queries");
DEFINE_int64(percentage_point_queries, 0, "% of Pointe Queries");
DEFINE_string(strategy, "raw", "Strategy");
DEFINE_string(distribution, "normal", "Random Distribution");
DEFINE_int64(iter, 0, "Define the iteration number");
DEFINE_int64(seed, 100, "Random Seed");
DEFINE_bool(write_disk, false, "Write output to disk");
string outputfolder = "output/";

using namespace std;

void printTimes(T * queriesMeta, Tree <Traits <T> > * & tree, double total_time, double mapping_time = 0) {
    double tbt, mt, t2t, ttt;
    tbt = total_time - tree->qMap->elapsedTime();
    T * leafsData = tree->getLeafsData();

    cout << FLAGS_iter << ",";
    cout << FLAGS_strategy << "," << FLAGS_distribution << "," << FLAGS_queries << ",";
    cout << FLAGS_key_domain_size << "," << FLAGS_leaf_size << "," << FLAGS_range_size << ",";
    cout << queriesMeta[0] << "," << queriesMeta[1] << "," << queriesMeta[2] << ",";
    cout << leafsData[0] << "," << leafsData[3] << "," << leafsData[4] << ",";
    cout << tree->qMap->csv() << ",";
    if (mapping_time > 0) {
        t2t = mapping_time;
        mt = 0;
        ttt = tbt + mapping_time;
    } else {
        t2t = 0;
        mt = tree->qMap->elapsedTime();
        ttt = total_time;
    }
    cout << tbt << "," << mt << "," << t2t << "," << ttt;
}

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
    cout << "," << elapsed_seconds.count();
    cout << "," << sum;
    cout << endl;
    return sum;
}


long additional(vector <Tinterval> & queries, rocksdb::DB* db, T leaf_size, T * queriesMeta) {
    long sum = 0;
    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());

    QMapExtra <Traits <T>> * qMap = new QMapExtra <Traits <T>>();
    Tree <Traits <T> > * tree = new Tree <Traits <T> >(leaf_size, qMap);

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

    et_1 = chrono::system_clock::now();
    elapsed_seconds = et_1 - st_1;
    mapping_time = elapsed_seconds.count();
    tree->qMap->indexed = leaftree.numIndexedQueries();

    vector <LeafNode<T> *> nodes = leaftree.nodes();
    sort(nodes.begin(), nodes.end(), compareInterval);

    for (auto itq = nodes.begin(); itq < nodes.end(); itq++) {
        auto st_2 = std::chrono::system_clock::now();
        Tinterval leaf = (*itq)->interval;
        string start = to_string(leaf.min);
        string limit = to_string(leaf.max);
        T ** temp = new T * [leaf.length()];

        for (int i = leaf.min; i < leaf.max; i += 1) {
            temp[i - leaf.min] = NULL;
        }
        for (it->Seek(start);
            it->Valid() && stoi(it->key().ToString()) < leaf.max;
            it->Next()
        ) {
            int key = stoi(it->key().ToString());
            temp[key - leaf.min] = new T(stoi(it->value().ToString()));
        }
        auto et_2 = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_timer = et_2 - st_2;
        db_exec_time += elapsed_timer.count();

        st_2 = std::chrono::system_clock::now();
        for(size_t i = 0; i < (*itq)->hashmap.size(); i++) {
            Tinterval * query = (*itq)->hashmap[i].first;
            Tinterval limits = (*itq)->hashmap[i].second;

        //     if (FLAGS_write_disk) {
        //         // string file_name = getFileName(*query);
        //         // ofstream ofile(file_name, std::ios_base::app);
        //         // for (T i = limits.min - leaf.min; i < (limits.max - leaf.min); i += 1) {
        //         //     sum += *temp[i];
        //         //     ofile << i + leaf.min << " - " << (*temp[i]) << "\n";
        //         // }
        //         // ofile.close();
        //     } else {
                for (T j = limits.min - leaf.min; j < (limits.max - leaf.min); j++) {
                    sum += (*temp[j]);
                }
        //     }
        }
        et_2 = std::chrono::system_clock::now();
        elapsed_timer = et_2 - st_2;
        post_filtering_time += elapsed_timer.count();
    }
    delete it;
    printTimes(queriesMeta, tree, tree_total_time, mapping_time);
    double totaltime = tree_total_time + mapping_time + db_exec_time + post_filtering_time;
    cout << "," << db_exec_time << "," << post_filtering_time << "," << totaltime;
    // cout << "," << sum;
    cout << endl;

    return sum;
}


long lazy(vector <Tinterval> & queries, rocksdb::DB* db, T leaf_size, T * queriesMeta) {
    long sum = 0;
    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());

    QMapLazy <Traits <T>> * qMap = new QMapLazy <Traits <T>>();
    Tree <Traits <T> > * tree = new Tree <Traits <T> >(leaf_size, qMap);

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

    auto st_2 = std::chrono::system_clock::now();
    for (auto itq = qMap->qMap.begin(); itq != qMap->qMap.end(); itq++) {

        Tinterval leaf = itq->first->interval;
        string start = to_string(leaf.min);
        string limit = to_string(leaf.max);
        T ** temp = new T * [leaf.length()];

        for (int i = leaf.min; i < leaf.max; i += 1) {
            temp[i - leaf.min] = NULL;
        }
        for (it->Seek(start);
            it->Valid() && stoi(it->key().ToString()) < leaf.max;
            it->Next()
        ) {
            int key = stoi(it->key().ToString());
            temp[key - leaf.min] = new T(stoll(it->value().ToString()));
        }

        for (size_t i = 0; i < itq->second.size(); i++) {
            Tinterval * query = itq->second[i];
            Tinterval limits = itq->first->interval.intersection(*query);

            if (limits.length() == 0) {
                continue;
            }

            if (FLAGS_write_disk) {
                string file_name = getFileName(*query);
                ofstream ofile(file_name, std::ios_base::app);
                for (T i = limits.min - leaf.min; i < (limits.max - leaf.min); i += 1) {
                    sum += *temp[i];
                    ofile << i + leaf.min << " - " << (*temp[i]) << "\n";
                }
                ofile.close();
            } else {
                T from = limits.min - leaf.min >= 0 ? limits.min - leaf.min : 0;
                T to = limits.max - leaf.min < leaf.max ? limits.max - leaf.min : leaf.max;

                for (T j = from; j < to; j++) {
                    sum += (*temp[j]);
                }
            }
        }
    }
    auto et_2 = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_timer = et_2 - st_2;
    db_exec_time = elapsed_timer.count();

    delete it;
    printTimes(queriesMeta, tree, tree_total_time);
    double totaltime = tree_total_time + db_exec_time + post_filtering_time;
    cout << "," << db_exec_time << "," << post_filtering_time << "," << totaltime;
    cout << "," << sum ;
    cout << endl;

    return sum;
}


long eager(vector <Tinterval> & queries, rocksdb::DB* db, T leaf_size, T * queriesMeta) {
    long sum = 0;
    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());

    QMapEager <Traits <T>> * qMap = new QMapEager <Traits <T>>();
    Tree <Traits <T> > * tree = new Tree <Traits <T>> (leaf_size, qMap);

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
        T ** temp = new T * [leaf.length()];

        for (int i = leaf.min; i < leaf.max; i += 1) {
            temp[i - leaf.min] = NULL;
        }
        for (it->Seek(start);
            it->Valid() && stoi(it->key().ToString()) < leaf.max;
            it->Next()
        ) {
            int key = stoi(it->key().ToString());
            temp[key - leaf.min] = new T(stoi(it->value().ToString()));
        }
        auto et_2 = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_timer = et_2 - st_2;
        db_exec_time += elapsed_timer.count();

        st_2 = std::chrono::system_clock::now();
        for (size_t i = 0; i < itq->second.size(); i++) {
            Tinterval * query = itq->second[i].first;
            Tinterval limits = itq->second[i].second;

            if (FLAGS_write_disk) {
                string file_name = getFileName(*query);
                ofstream ofile(file_name, std::ios_base::app);
                for (T i = limits.min - leaf.min; i < (limits.max - leaf.min); i += 1) {
                    sum += *temp[i];
                    ofile << i + leaf.min << " - " << (*temp[i]) << "\n";
                }
                ofile.close();
            } else {
                T from = limits.min - leaf.min >= 0 ? limits.min - leaf.min : 0;
                T to = limits.max - leaf.min < leaf.max ? limits.max - leaf.min : leaf.max;

                for (T j = from; j < to; j++) {
                    sum += (*temp[j]);
                }
            }

        }
        et_2 = std::chrono::system_clock::now();
        elapsed_timer = et_2 - st_2;
        post_filtering_time += elapsed_timer.count();
    }
    delete it;
    printTimes(queriesMeta, tree, tree_total_time);
    double totaltime = tree_total_time + db_exec_time + post_filtering_time;
    cout << "," << db_exec_time << "," << post_filtering_time << "," << totaltime;
    // cout << "," << sum ;
    cout << endl;

    return sum;
}


int main(int argc, char** argv) {
    srand (FLAGS_seed);

    gflags::SetUsageMessage("Main");
    gflags::SetVersionString("1.0.0");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    vector <Tinterval> queries;

    if (FLAGS_distribution == "zipf") {
        queries = create_queries_zipf(FLAGS_queries, FLAGS_key_domain_size, FLAGS_range_size, FLAGS_random_range_size, FLAGS_min_range_size, FLAGS_max_range_size, FLAGS_percentage_point_queries);
    } else {
        queries = create_queries(FLAGS_queries, FLAGS_key_domain_size, FLAGS_range_size, FLAGS_random_range_size, FLAGS_min_range_size, FLAGS_max_range_size, FLAGS_percentage_point_queries);
    }

    rocksdb::DB* db;
    rocksdb::Options options;
    options.create_if_missing = true;
    NumericComparator cmp;
    options.comparator = &cmp;

    rocksdb::Status status = rocksdb::DB::Open(options, "/dev/shm", &db);
    assert(status.ok());

    long sum = 0;

    T leaf_size = 0;

    T * queriesMeta = getQueriesMeta(queries);

    if (is_number(FLAGS_leaf_size)) {
        leaf_size = atol(FLAGS_leaf_size.c_str());
    } else if (FLAGS_leaf_size == "max_range") {
        leaf_size = queriesMeta[2];
    }

    if (FLAGS_strategy == "lazy") {
        sum = lazy(queries, db, leaf_size, queriesMeta);
    } else if (FLAGS_strategy == "original") {
        sum = original(queries, db);
    } else if (FLAGS_strategy == "additional") {
        sum = additional(queries, db, leaf_size, queriesMeta);
    } else if (FLAGS_strategy == "eager") {
        sum = eager(queries, db, leaf_size, queriesMeta);
    }

    delete db;

    gflags::ShutDownCommandLineFlags();

    return 0;
}
