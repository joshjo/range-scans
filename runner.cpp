#include <gflags/gflags.h>

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "numeric_comparator.h"
#include "result.h"
#include "utils.h"
#include "../interval-base-tree/src/newtree.h"

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



double printTimes(T * queriesMeta, Tree <Traits <T> > * & tree, double total_time, double mapping_time = 0) {
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

    return ttt;
}

// void printTimes(T * queriesMeta, Tree <Traits <T> > * & tree, double total_time, double mapping_time = 0) {
//     double tbt, mt, t2t, ttt;
//     tbt = total_time - tree->qMap->elapsedTime();
//     T * leafsData = tree->getLeafsData();
//     vector<Node<T> *> leafs;
//     tree->root->getLeafs(leafs);


//     cout << "queries        : " << FLAGS_queries << endl;
//     cout << "avg rangesize  : " << queriesMeta[1] << endl;
//     cout << "avg node length: " << leafsData[0] << endl;
//     cout << "leaf nodes     : " << leafsData[3] << endl;
//     cout << "leaf size      : " << FLAGS_leaf_size << endl;
//     // cout << FLAGS_iter << ",";
//     // cout << FLAGS_strategy << "," << FLAGS_distribution << "," << FLAGS_queries << ",";
//     // cout << FLAGS_key_domain_size << "," << FLAGS_leaf_size << "," << FLAGS_range_size << ",";
//     // cout << queriesMeta[0] << "," << queriesMeta[1] << "," << queriesMeta[2] << ",";
//     // cout << leafsData[0] << "," << leafsData[3] << "," << leafsData[4] << ",";
//     // cout << tree->qMap->csv() << ",";
//     // if (mapping_time > 0) {
//     //     t2t = mapping_time;
//     //     mt = 0;
//     //     ttt = tbt + mapping_time;
//     // } else {
//     //     t2t = 0;
//     //     mt = tree->qMap->elapsedTime();
//     //     ttt = total_time;
//     // }
//     // cout << tbt << "," << mt << "," << t2t << "," << ttt;
// }

bool compareInterval(LeafNode<T> * i1, LeafNode<T> * i2)
{
    return (i1->interval.min < i2->interval.min);
}

void QAT(vector <Tinterval> & queries, T * queriesMeta) {
    rocksdb::DB* db;
    rocksdb::Options options;
    options.create_if_missing = true;
    NumericComparator cmp;
    options.comparator = &cmp;

    rocksdb::Status status = rocksdb::DB::Open(options, "/tmp/tree", &db);
    assert(status.ok());

    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    long checksum = 0;
    auto st_1 = chrono::system_clock::now();
    for (int i = 0; i < queries.size(); i += 1) {
        Tinterval interval = queries[i];
        string start = to_string(interval.min);
        string limit = to_string(interval.max);
        if (FLAGS_write_disk) {
            // string file_name = getFileName(interval);
            // ofstream ofile(file_name);
            // for (it->Seek(start);
            //     it->Valid() && stoi(it->key().ToString()) < interval.max;
            //     it->Next())
            // {
            //     checksum += stoi(it->value().ToString());
            //     ofile << it->key().ToString() << " - " << it->value().ToString() << "\n";
            // }
            // ofile.close();
        } else {
            for (it->Seek(start);
                it->Valid() && stoi(it->key().ToString()) < interval.max;
                it->Next())
            {
                // it->value();
                // cout << it->value() << endl;
                string parts = it->value().ToString();
                // atoi(parts.c_str());
                // it->value();
                stoi(parts);
            }
        }
    }
    auto et_1 = chrono::system_clock::now();
    delete it;
    delete db;

    chrono::duration<double> elapsed_seconds = et_1 - st_1;
    cout << FLAGS_iter << "," << FLAGS_distribution << "," << queriesMeta[1] << "," << FLAGS_queries;
    cout << "," << elapsed_seconds.count() << endl;
}


void additionalPostProcessing(
        vector <Tinterval> & queries,
        Tree <Traits <T> > * tree,
        LeafTree<Traits <T>> & leaftree,
        double ttt) {

    rocksdb::DB* db;
    rocksdb::Options options;
    options.create_if_missing = true;
    NumericComparator cmp;
    options.comparator = &cmp;

    rocksdb::Status status = rocksdb::DB::Open(options, "/tmp/tree", &db);
    assert(status.ok());

    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());

    double db_exec_time = 0;
    double post_filtering_time = 0;
    unsigned long long int sum = 0;

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
            temp[key-leaf.min] = new T(stoll(it->value().ToString()));
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
                    // *temp[j];
                    *temp[j];
                }
            }

        }
        et_2 = std::chrono::system_clock::now();
        elapsed_timer = et_2 - st_2;
        post_filtering_time += elapsed_timer.count();
    }

    cout << "," << post_filtering_time << "," << db_exec_time << "," << post_filtering_time + db_exec_time + ttt;

    delete it;
    delete db;

}

void eagerPostProcessing(
        vector <Tinterval> & queries,
        Tree <Traits <T> > * tree,
        QMapEager <Traits <T>> * qMap,
        double ttt
    ){
    rocksdb::DB* db;
    rocksdb::Options options;
    options.create_if_missing = true;
    NumericComparator cmp;
    options.comparator = &cmp;

    rocksdb::Status status = rocksdb::DB::Open(options, "/tmp/tree", &db);
    assert(status.ok());

    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());

    double db_exec_time = 0;
    double post_filtering_time = 0;
    unsigned long long int sum = 0;

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
                    stoi(*temp[j]);
                }
            }

        }
        et_2 = std::chrono::system_clock::now();
        elapsed_timer = et_2 - st_2;
        post_filtering_time += elapsed_timer.count();
    }

    cout << "," << post_filtering_time << "," << db_exec_time << "," << post_filtering_time + db_exec_time + ttt;
    delete it;
    delete db;
}

void lazyPostProcessing(
    vector <Tinterval> & queries,
    Tree <Traits <T> > * tree,
    QMapLazy <Traits <T>> * qMap,
        double ttt
){
    rocksdb::DB* db;
    rocksdb::Options options;
    options.create_if_missing = true;
    NumericComparator cmp;
    options.comparator = &cmp;

    rocksdb::Status status = rocksdb::DB::Open(options, "/tmp/tree", &db);
    assert(status.ok());

    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());

    double db_exec_time = 0;
    double post_filtering_time = 0;
    unsigned long long int sum = 0;

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
                    stoi(*temp[j]);
                }
            }

        }
        et_2 = std::chrono::system_clock::now();
        elapsed_timer = et_2 - st_2;
        post_filtering_time += elapsed_timer.count();
    }

    cout << "," << post_filtering_time << "," << db_exec_time << "," << post_filtering_time + db_exec_time + ttt;

    delete it;
    delete db;
}

void additional(vector <Tinterval> & queries, T leaf_size, T * queriesMeta) {
    auto start_time = std::chrono::system_clock::now();
    QMapExtra <Traits <T>> * qMap = new QMapExtra <Traits <T>>();
    Tree <Traits <T> > * tree = new Tree <Traits <T> >(leaf_size, qMap);

    for (int i = 0; i < queries.size(); i += 1) {
        tree->insert(queries[i]);
    }

    auto end_time = std::chrono::system_clock::now();
    chrono::duration<double> elapsed_seconds = end_time - start_time;
    double total_time = elapsed_seconds.count();

    start_time = std::chrono::system_clock::now();
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
    tree->qMap->indexed = leaftree.numIndexedQueries();

    end_time = std::chrono::system_clock::now();
    elapsed_seconds = end_time - start_time;
    double mapping_time = elapsed_seconds.count();

    double ttt = printTimes(queriesMeta, tree, total_time, mapping_time);
    additionalPostProcessing(queries, tree, leaftree, ttt);

    cout << endl;
}

void eager(vector <Tinterval> & queries, T leaf_size, T * queriesMeta) {
    auto start_time = std::chrono::system_clock::now();
    QMapEager <Traits <T>> * qMap = new QMapEager <Traits <T>>();
    Tree <Traits <T> > * tree = new Tree <Traits <T> >(leaf_size, qMap);

    for (int i = 0; i < queries.size(); i += 1) {
        tree->insert(queries[i]);
    }

    auto end_time = std::chrono::system_clock::now();
    chrono::duration<double> elapsed_seconds = end_time - start_time;
    double total_time = elapsed_seconds.count();

    vector<Node<T> *> leafs;
    tree->root->getLeafs(leafs);
    tree->root->recursiveValidate();

    double ttt = printTimes(queriesMeta, tree, total_time);
    eagerPostProcessing(queries, tree, qMap, ttt);
    cout << endl;
}


void lazy(vector <Tinterval> & queries, T leaf_size, T * queriesMeta) {
    cout << "heereee" << endl;
    auto start_time = std::chrono::system_clock::now();
    QMapLazy <Traits <T>> * qMap = new QMapLazy <Traits <T>>();
    Tree <Traits <T> > * tree = new Tree <Traits <T> >(leaf_size, qMap);

    for (int i = 0; i < queries.size(); i += 1) {
        tree->insert(queries[i]);
    }

    auto end_time = std::chrono::system_clock::now();
    chrono::duration<double> elapsed_seconds = end_time - start_time;
    double total_time = elapsed_seconds.count();

    double ttt = printTimes(queriesMeta, tree, total_time);
    // lazyPostProcessing(queries, tree, qMap, ttt);
    cout << endl;
}

int main(int argc, char** argv) {
    gflags::SetUsageMessage("Profiling");
    gflags::SetVersionString("1.0.0");
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    if (FLAGS_range_size > FLAGS_key_domain_size) {
        cout << "Range Size should be lower that domain" << endl;
        return 0;
    }

    srand (FLAGS_seed);
    vector <Tinterval> queries;
    T leaf_size = 0;

    if (FLAGS_distribution == "zipf") {
        queries = create_queries_zipf(FLAGS_queries, FLAGS_key_domain_size, FLAGS_range_size, FLAGS_random_range_size, FLAGS_min_range_size, FLAGS_max_range_size, FLAGS_percentage_point_queries);
    } else {
        queries = create_queries(FLAGS_queries, FLAGS_key_domain_size, FLAGS_range_size, FLAGS_random_range_size, FLAGS_min_range_size, FLAGS_max_range_size, FLAGS_percentage_point_queries);
    }

    T * queriesMeta = getQueriesMeta(queries);

    if (is_number(FLAGS_leaf_size)) {
        leaf_size = atol(FLAGS_leaf_size.c_str());
    } else if (FLAGS_leaf_size == "max_range") {
        leaf_size = queriesMeta[2];
    }

    // QAT(queries);

    if (FLAGS_strategy == "lazy") {
        lazy(queries, leaf_size, queriesMeta);
    } else if (FLAGS_strategy == "eager") {
        eager(queries, leaf_size, queriesMeta);
    } else if (FLAGS_strategy == "additional") {
        additional(queries, leaf_size, queriesMeta);
    } else {
        QAT(queries, queriesMeta);
    }

    gflags::ShutDownCommandLineFlags();
    return 0;
}
