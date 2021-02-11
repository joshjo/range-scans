#include <assert.h>
#include <iostream>
#include <fstream>
#include <typeinfo>
#include <chrono>
#include <ctime>
#include <random>
#include <gflags/gflags.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "numeric_comparator.h"

#include "src/query.h"
#include "src/citree.h"
#include "src/aitree.h"
#include "src/uitree.h"
#include "src/tools.h"

typedef long long T;
typedef Interval<T> Tinterval;
typedef Query<T> Tquery;

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


string getFileName(Tinterval & interval) {
    string folder_path = outputfolder + FLAGS_strategy;
    string file_name = folder_path + "/interval-";
    mkdir(folder_path.c_str(), 0755);
    file_name += to_string(interval.min) + "-" + to_string(interval.max) + ".txt";
    return file_name;
}


void printTimes(T * queriesMeta, UITree <Traits <T> > * & uitree, double total_time, double mapping_time = 0) {
    double tbt, mt, t2t, ttt;
    tbt = total_time - uitree->qMap->elapsedTime();
    T * leafsData = uitree->getLeafsData();
    // cout << queriesMeta[0] << "," << queriesMeta[1] << "," << queriesMeta[2] << ",";
    // cout << leafsData[0] << "," << leafsData[3] << "," << leafsData[4] << ",";
    cout << leafsData[0] << ",";
    // cout << uitree->qMap->csv() << ",";
    if (mapping_time > 0) {
        t2t = mapping_time;
        mt = 0;
        ttt = tbt + mapping_time;
    } else {
        t2t = 0;
        mt = uitree->qMap->elapsedTime();
        ttt = total_time;
    }
    cout << tbt << "," << mt << "," << t2t << "," << ttt << ",";
    // cout << FLAGS_iter << ",";
    cout << FLAGS_strategy << "," << FLAGS_distribution << "," << FLAGS_queries << ",";
    cout << FLAGS_key_domain_size << "," << FLAGS_leaf_size << "," << FLAGS_range_size << ",";
}


long original(vector <Tquery *> & queries, rocksdb::DB* db) {
    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    long sum = 0;
    auto st_1 = chrono::system_clock::now();
    for (int i = 0; i < queries.size(); i += 1) {
        Tinterval interval = queries[i]->interval;
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

long lazy(vector <Tquery *> & queries, rocksdb::DB* db, T leaf_size, T * queriesMeta) {
    long sum = 0;
    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());

    QMapLazy <Traits <T>> * qMap = new QMapLazy <Traits <T>>();
    UITree <Traits <T> > * uitree = new UITree <Traits <T> >(leaf_size, qMap);

    double uitree_time = 0;
    double post_filtering_time = 0;
    double db_and_postfiltering = 0;
    double db_exec_time = 0;
    double pf_tree_building = 0;
    double pf_tree_find = 0;
    double query_assignment = 0;

    auto st_1 = chrono::system_clock::now();
    for (int i = 0; i < queries.size(); i += 1) {
        uitree->insert(queries[i]);
    }
    auto et_1 = chrono::system_clock::now();
    chrono::duration<double> e1 = et_1 - st_1;
    uitree_time = e1.count();

    auto st_2 = std::chrono::system_clock::now();
    for (auto itq = qMap->qMap.begin(); itq != qMap->qMap.end(); itq++) {

        Tinterval leaf = itq->first->interval;
        string start = to_string(leaf.min);
        string limit = to_string(leaf.max);

        auto st_3 = std::chrono::system_clock::now();
        T ** temp = new T * [leaf.length()];
        for (int i = leaf.min; i < leaf.max; i += 1) {
            temp[i - leaf.min] = NULL;
        }
        auto et_3 = chrono::system_clock::now();
        chrono::duration<double> e3 = et_3 - st_3;
        post_filtering_time += e3.count();

        for (it->Seek(start);
            it->Valid() && stoi(it->key().ToString()) < leaf.max;
            it->Next()
        ) {
            T key = stoll(it->key().ToString());
            T value = stoll(it->value().ToString());

            auto st_4 = std::chrono::system_clock::now();
            temp[key - leaf.min] = new T(value);
            auto et_4 = chrono::system_clock::now();
            chrono::duration<double> e4 = et_4 - st_4;

            post_filtering_time += e4.count();
        }

        for (size_t i = 0; i < itq->second.size(); i++) {
            Tquery * query = itq->second[i];
            Tinterval limits = itq->first->interval.intersection(query->interval);

            if (limits.length() == 0) {
                continue;
            }

            auto st_5 = std::chrono::system_clock::now();
            if (FLAGS_write_disk) {
                string file_name = getFileName(query->interval);
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
            auto et_5 = chrono::system_clock::now();
            chrono::duration<double> e5 = et_5 - st_5;
            query_assignment += e5.count();
        }
    }
    auto et_2 = std::chrono::system_clock::now();
    std::chrono::duration<double> e2 = et_2 - st_2;
    db_and_postfiltering = e2.count();
    post_filtering_time = post_filtering_time + query_assignment;
    db_exec_time = db_and_postfiltering - post_filtering_time;

    delete it;
    cout << post_filtering_time << "," << pf_tree_building << "," << pf_tree_find << ",";
    cout << query_assignment << "," << db_exec_time << ",";
    printTimes(queriesMeta, uitree, uitree_time);
    cout << sum;
    cout << endl;

    return sum;
}

long eager(vector <Tquery *> & queries, rocksdb::DB* db, T leaf_size, T * queriesMeta) {
    long sum = 0;
    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());

    QMapEager <Traits <T>> * qMap = new QMapEager <Traits <T>>();
    UITree <Traits <T> > * uitree = new UITree <Traits <T> >(leaf_size, qMap);

    double uitree_time = 0;
    double post_filtering_time = 0;
    double db_and_postfiltering = 0;
    double db_exec_time = 0;
    double pf_tree_building = 0;
    double pf_tree_find = 0;
    double query_assignment = 0;

    auto st_1 = chrono::system_clock::now();
    for (int i = 0; i < queries.size(); i += 1) {
        uitree->insert(queries[i]);
    }
    auto et_1 = chrono::system_clock::now();
    chrono::duration<double> e1 = et_1 - st_1;
    uitree_time = e1.count();

    auto st_2 = std::chrono::system_clock::now();
    for (auto itq = qMap->qMap.begin(); itq != qMap->qMap.end(); itq++) {

        Tinterval leaf = itq->first->interval;
        string start = to_string(leaf.min);
        string limit = to_string(leaf.max);

        auto st_3 = std::chrono::system_clock::now();
        T ** temp = new T * [leaf.length()];
        for (int i = leaf.min; i < leaf.max; i += 1) {
            temp[i - leaf.min] = NULL;
        }
        auto et_3 = chrono::system_clock::now();
        chrono::duration<double> e3 = et_3 - st_3;
        post_filtering_time += e3.count();

        for (it->Seek(start);
            it->Valid() && stoi(it->key().ToString()) < leaf.max;
            it->Next()
        ) {
            T key = stoll(it->key().ToString());
            T value = stoll(it->value().ToString());

            auto st_4 = std::chrono::system_clock::now();
            temp[key - leaf.min] = new T(value);
            auto et_4 = chrono::system_clock::now();
            chrono::duration<double> e4 = et_4 - st_4;

            post_filtering_time += e4.count();
        }

        for (size_t i = 0; i < itq->second.size(); i++) {
            Tquery * query = itq->second[i];
            Tinterval limits = itq->first->interval.intersection(query->interval);

            if (limits.length() == 0) {
                continue;
            }

            auto st_5 = std::chrono::system_clock::now();
            if (FLAGS_write_disk) {
                string file_name = getFileName(query->interval);
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
            auto et_5 = chrono::system_clock::now();
            chrono::duration<double> e5 = et_5 - st_5;
            query_assignment += e5.count();
        }
    }
    auto et_2 = std::chrono::system_clock::now();
    std::chrono::duration<double> e2 = et_2 - st_2;
    db_and_postfiltering = e2.count();
    post_filtering_time = post_filtering_time + query_assignment;
    db_exec_time = db_and_postfiltering - post_filtering_time;

    delete it;
    cout << post_filtering_time << "," << pf_tree_building << "," << pf_tree_find << ",";
    cout << query_assignment << "," << db_exec_time << ",";
    printTimes(queriesMeta, uitree, uitree_time);
    cout << sum;
    cout << endl;

    return sum;
}

long lazyop(vector <Tquery *> & queries, rocksdb::DB* db, T leaf_size, T * queriesMeta) {
    long sum = 0;
    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());

    QMapLazy <Traits <T>> * qMap = new QMapLazy <Traits <T>>();
    UITree <Traits <T> > * uitree = new UITree <Traits <T> >(leaf_size, qMap);

    double uitree_time = 0;
    double post_filtering_time = 0;
    double db_and_postfiltering = 0;
    double db_exec_time = 0;
    double pf_tree_building = 0;
    double pf_tree_find = 0;
    double query_assignment = 0;

    auto st_1 = chrono::system_clock::now();
    for (int i = 0; i < queries.size(); i += 1) {
        uitree->insert(queries[i]);
    }
    auto et_1 = chrono::system_clock::now();
    chrono::duration<double> e1 = et_1 - st_1;
    uitree_time = e1.count();

    auto st_2 = std::chrono::system_clock::now();
    for (auto itq = qMap->qMap.begin(); itq != qMap->qMap.end(); itq++) {

        auto st_3 = std::chrono::system_clock::now();
        CITree <T> qtree;
        qtree.insert(itq->second);
        auto et_3 = chrono::system_clock::now();
        chrono::duration<double> e3 = et_3 - st_3;
        pf_tree_building += e3.count();

        // cout << qtree.graphviz() << endl << endl;

        Tinterval leaf = itq->first->interval;
        string start = to_string(leaf.min);
        string limit = to_string(leaf.max);

        for (it->Seek(start);
            it->Valid() && stoi(it->key().ToString()) < leaf.max;
            it->Next()
        ) {
            T key = stoll(it->key().ToString());
            T value = stoll(it->value().ToString());

            auto st_4 = std::chrono::system_clock::now();
            vector <Tquery *> intersectedQueries = qtree.find(key);
            auto et_4 = chrono::system_clock::now();
            chrono::duration<double> e4 = et_4 - st_4;
            pf_tree_find += e4.count();

            auto st_5 = std::chrono::system_clock::now();
            for (size_t s = 0; s < intersectedQueries.size(); s++) {
                sum += value;
            }
            auto et_5 = chrono::system_clock::now();
            chrono::duration<double> e5 = et_5 - st_5;
            query_assignment += e5.count();
        }
    }
    auto et_2 = std::chrono::system_clock::now();
    std::chrono::duration<double> e2 = et_2 - st_2;
    db_and_postfiltering = e2.count();

    post_filtering_time = pf_tree_building + pf_tree_find + query_assignment;

    delete it;
    db_exec_time = db_and_postfiltering - post_filtering_time;
    cout << post_filtering_time << "," << pf_tree_building << "," << pf_tree_find << ",";
    cout << query_assignment << "," << db_exec_time << ",";
    printTimes(queriesMeta, uitree, uitree_time);
    cout << sum;
    cout << endl;

    return sum;
}


long lazynormal(vector <Tquery *> & queries, rocksdb::DB* db, T leaf_size, T * queriesMeta) {
    long sum = 0;
    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());

    QMapLazy <Traits <T>> * qMap = new QMapLazy <Traits <T>>();
    UITree <Traits <T> > * uitree = new UITree <Traits <T> >(leaf_size, qMap);

    double uitree_time = 0;
    double post_filtering_time = 0;
    double db_and_postfiltering = 0;
    double db_exec_time = 0;
    double pf_tree_building = 0;
    double pf_tree_find = 0;
    double query_assignment = 0;

    auto st_1 = chrono::system_clock::now();
    for (int i = 0; i < queries.size(); i += 1) {
        uitree->insert(queries[i]);
    }
    auto et_1 = chrono::system_clock::now();
    chrono::duration<double> e1 = et_1 - st_1;
    uitree_time = e1.count();

    auto st_2 = std::chrono::system_clock::now();
    for (auto itq = qMap->qMap.begin(); itq != qMap->qMap.end(); itq++) {

        // auto st_3 = std::chrono::system_clock::now();
        // AITree <T> qtree;
        // qtree.insert(itq->second);
        // auto et_3 = chrono::system_clock::now();
        // chrono::duration<double> e3 = et_3 - st_3;
        // pf_tree_building += e3.count();

        // cout << qtree.graphviz() << endl << endl;

        Tinterval leaf = itq->first->interval;
        string start = to_string(leaf.min);
        string limit = to_string(leaf.max);

        for (it->Seek(start);
            it->Valid() && stoi(it->key().ToString()) < leaf.max;
            it->Next()
        ) {
            T key = stoll(it->key().ToString());
            T value = stoll(it->value().ToString());

            auto st_4 = std::chrono::system_clock::now();
            vector <Tquery *> intersectedQueries;
            for (size_t i = 0; i < itq->second.size(); i++) {
                if(itq->second[i]->interval.intersects(key)) {
                    intersectedQueries.push_back(itq->second[i]);
                }
            }
            auto et_4 = chrono::system_clock::now();
            chrono::duration<double> e4 = et_4 - st_4;
            pf_tree_find += e4.count();

            auto st_5 = std::chrono::system_clock::now();
            for (size_t s = 0; s < intersectedQueries.size(); s++) {
                sum += value;
            }
            auto et_5 = chrono::system_clock::now();
            chrono::duration<double> e5 = et_5 - st_5;
            query_assignment += e5.count();
        }
    }
    auto et_2 = std::chrono::system_clock::now();
    std::chrono::duration<double> e2 = et_2 - st_2;
    db_and_postfiltering = e2.count();

    post_filtering_time = pf_tree_building + pf_tree_find + query_assignment;

    delete it;
    db_exec_time = db_and_postfiltering - post_filtering_time;
    cout << post_filtering_time << "," << pf_tree_building << "," << pf_tree_find << ",";
    cout << query_assignment << "," << db_exec_time << ",";
    printTimes(queriesMeta, uitree, uitree_time);
    cout << sum;
    cout << endl;

    return sum;
}

long eagerop(vector <Tquery *> & queries, rocksdb::DB* db, T leaf_size, T * queriesMeta) {
    long sum = 0;
    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());

    QMapEager <Traits <T>> * qMap = new QMapEager <Traits <T>>();
    UITree <Traits <T> > * uitree = new UITree <Traits <T> >(leaf_size, qMap);

    double uitree_time = 0;
    double post_filtering_time = 0;
    double db_and_postfiltering = 0;
    double db_exec_time = 0;
    double pf_tree_building = 0;
    double pf_tree_find = 0;
    double query_assignment = 0;

    auto st_1 = chrono::system_clock::now();
    for (int i = 0; i < queries.size(); i += 1) {
        uitree->insert(queries[i]);
    }
    auto et_1 = chrono::system_clock::now();
    chrono::duration<double> e1 = et_1 - st_1;
    uitree_time = e1.count();

    auto st_2 = std::chrono::system_clock::now();
    for (auto itq = qMap->qMap.begin(); itq != qMap->qMap.end(); itq++) {

        auto st_3 = std::chrono::system_clock::now();
        CITree <T> qtree;
        qtree.insert(itq->second);
        auto et_3 = chrono::system_clock::now();
        chrono::duration<double> e3 = et_3 - st_3;
        pf_tree_building += e3.count();

        Tinterval leaf = itq->first->interval;
        string start = to_string(leaf.min);
        string limit = to_string(leaf.max);

        cout << qtree.graphviz() << endl << endl;

        for (it->Seek(start);
            it->Valid() && stoi(it->key().ToString()) < leaf.max;
            it->Next()
        ) {
            T key = stoll(it->key().ToString());
            T value = stoll(it->value().ToString());

            auto st_4 = std::chrono::system_clock::now();
            vector <Tquery *> intersectedQueries = qtree.find(key);
            auto et_4 = chrono::system_clock::now();
            chrono::duration<double> e4 = et_4 - st_4;
            pf_tree_find += e4.count();

            auto st_5 = std::chrono::system_clock::now();
            for (size_t s = 0; s < intersectedQueries.size(); s++) {
                sum += value;
            }
            auto et_5 = chrono::system_clock::now();
            chrono::duration<double> e5 = et_5 - st_5;
            query_assignment += e5.count();
        }
    }
    auto et_2 = std::chrono::system_clock::now();
    std::chrono::duration<double> e2 = et_2 - st_2;
    db_and_postfiltering = e2.count();

    post_filtering_time = pf_tree_building + pf_tree_find + query_assignment;

    delete it;
    db_exec_time = db_and_postfiltering - post_filtering_time;
    cout << post_filtering_time << "," << pf_tree_building << "," << pf_tree_find << ",";
    cout << query_assignment << "," << db_exec_time << ",";
    printTimes(queriesMeta, uitree, uitree_time);
    cout << sum;
    cout << endl;

    return sum;
}


int main(int argc, char** argv) {
    srand (FLAGS_seed);

    gflags::SetUsageMessage("Main");
    gflags::SetVersionString("1.0.0");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    vector <Tquery *> queries;

    queries = create_random_queries(
        FLAGS_queries,
        FLAGS_key_domain_size,
        FLAGS_range_size,
        FLAGS_random_range_size,
        FLAGS_min_range_size,
        FLAGS_max_range_size,
        FLAGS_percentage_point_queries,
        FLAGS_seed
    );


    // if (FLAGS_distribution == "zipf") {
    //     queries = create_queries_zipf(FLAGS_queries, FLAGS_key_domain_size, FLAGS_range_size, FLAGS_random_range_size, FLAGS_min_range_size, FLAGS_max_range_size, FLAGS_percentage_point_queries);
    // } else {
    //     queries = create_queries(FLAGS_queries, FLAGS_key_domain_size, FLAGS_range_size, FLAGS_random_range_size, FLAGS_min_range_size, FLAGS_max_range_size, FLAGS_percentage_point_queries);
    // }

    rocksdb::DB* db;
    rocksdb::Options options;
    options.create_if_missing = true;
    NumericComparator cmp;
    options.comparator = &cmp;

    rocksdb::Status status = rocksdb::DB::Open(options, "/tmp/tree", &db);
    assert(status.ok());

    long sum = 0;

    T leaf_size = 0;

    T * queriesMeta = getQueriesMetaData(queries);

    long long int cheks = getFastQueriesChecksum(queries);

    if (isNumber(FLAGS_leaf_size)) {
        leaf_size = atol(FLAGS_leaf_size.c_str());
    } else if (FLAGS_leaf_size == "max_range") {
        leaf_size = queriesMeta[2];
    }

    if (FLAGS_strategy == "original") {
        original(queries, db);
    } else if (FLAGS_strategy == "lazyop") {
        lazyop(queries, db, leaf_size, queriesMeta);
    } else if (FLAGS_strategy == "lazynormal") {
        lazynormal(queries, db, leaf_size, queriesMeta);
    } else if (FLAGS_strategy == "lazy") {
        lazy(queries, db, leaf_size, queriesMeta);
    } else if (FLAGS_strategy == "eager") {
        eager(queries, db, leaf_size, queriesMeta);
    } else if (FLAGS_strategy == "eagerop") {
        eagerop(queries, db, leaf_size, queriesMeta);
    }


    // if (FLAGS_strategy == "lazyold") {
    //     sum = lazyold(queries, db, leaf_size, queriesMeta);
    // } if (FLAGS_strategy == "lazy") {
    //     sum = lazy(queries, db, leaf_size, queriesMeta);
    // } else if (FLAGS_strategy == "original") {
    //     sum = original(queries, db);
    // } else if (FLAGS_strategy == "additional") {
    //     sum = additional(queries, db, leaf_size, queriesMeta);
    // } else if (FLAGS_strategy == "eager") {
    //     sum = eager(queries, db, leaf_size, queriesMeta);
    // }

    // cout << "cheks: " << cheks;

    delete db;

    gflags::ShutDownCommandLineFlags();

    return 0;
}
