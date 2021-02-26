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

#include <sqlite3.h>

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "numeric_comparator.h"

#include "src/query.h"
#include "src/citree.h"
#include "src/aitree.h"
#include "src/uitree.h"
#include "src/tools.h"
#include "src/result.h"
#include "duckdb.hpp"

typedef long long T;
typedef Interval<T> Tinterval;
typedef Query<T> Tquery;
typedef vector <Tquery *> qArray;
typedef Result <T> Tresult;
typedef unordered_map<Tquery *, qArray> queryMap;

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

using namespace std;

vector<Tresult *> executeRocksDBQueries(vector <Tquery *> & queryPlan) {
    vector <Tresult *> table;
    rocksdb::DB* db;
    rocksdb::Options options;
    NumericComparator cmp;
    options.comparator = &cmp;
    T checksum = 0;
    rocksdb::Status status = rocksdb::DB::Open(options, "/dev/shm", &db);
    assert(status.ok());
    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());

    for(size_t i = 0; i < queryPlan.size(); i++) {
        Tinterval leaf = queryPlan[i]->interval;
        string start = to_string(leaf.min);
        string limit = to_string(leaf.max);
        Tresult * result = new Tresult(queryPlan[i]);
         for (it->Seek(start);
            it->Valid() && stoi(it->key().ToString()) < leaf.max;
            it->Next()
        ) {
            T key = stoll(it->key().ToString());
            T val = stoll(it->value().ToString());
            result->appendResult(key, val);
        }
        table.push_back(result);
    }
    delete db;
    return table;
}

void executeDucksDBQueries(vector <Tquery *> & queryPlan) {
    duckdb::DuckDB db("/dev/shm/josue.db");
	duckdb::Connection con(db);
    auto st2 = std::chrono::system_clock::now();
    char buffer[100];
    T checksum = 0;
    for(size_t i = 0; i < queryPlan.size(); i++) {
        Tinterval leaf = queryPlan[i]->interval;
        sprintf(buffer, "SELECT * FROM simple WHERE key >= %lld AND key < %lld", leaf.min, leaf.max);
        auto result = con.Query(buffer);
        for (size_t r = 0; r < result->collection.Count(); r++) {
            T val = result->GetValue<int64_t>(1, r);
            checksum += val;
        }
    }
    auto et2 = std::chrono::system_clock::now();
    chrono::duration<double> e2 = et2 - st2;
    cout << FLAGS_queries << "," << e2.count() << "," << checksum << endl;
    // cout << "t2: " << e2.count() << endl;
    // cout << "checksum: " << checksum << endl;
}

void executeAndPostFilteringDucksDBQueries(QMapLazy <Traits <T>> * qMap) {
    duckdb::DuckDB db("/dev/shm/josue.db");
	duckdb::Connection con(db);
    double dbtime = 0;
    double pftime = 0;
    auto st2 = std::chrono::system_clock::now();
    char buffer[100];
    T checksum = 0;
    for(auto itq = qMap->qMap.begin(); itq != qMap->qMap.end(); itq++) {
        auto st1 = std::chrono::system_clock::now();
        Tinterval leaf = itq->first->interval;
        sprintf(buffer, "SELECT * FROM simple WHERE key >= %lld AND key < %lld", leaf.min, leaf.max);
        auto result = con.Query(buffer);
        auto et1 = std::chrono::system_clock::now();
        chrono::duration<double> e1 = et1 - st1;
        dbtime += e1.count();

        for (size_t r = 0; r < result->collection.Count(); r++) {
            auto st3 = std::chrono::system_clock::now();
            T key = result->GetValue<int64_t>(0, r);
            T val = result->GetValue<int64_t>(1, r);
            auto et3 = std::chrono::system_clock::now();
            chrono::duration<double> e3 = et3 - st3;
            dbtime += e3.count();

            auto st4 = std::chrono::system_clock::now();
            vector <Tquery *> intersectedQueries;
            for (size_t i = 0; i < itq->second.size(); i++) {
                if(itq->second[i]->interval.intersects(key)) {
                    intersectedQueries.push_back(itq->second[i]);
                }
            }
            for (size_t s = 0; s < intersectedQueries.size(); s++) {
                checksum += val;
            }
            auto et4 = std::chrono::system_clock::now();
            chrono::duration<double> e4 = et4 - st4;
            pftime += e4.count();
        }
    }
    auto et2 = std::chrono::system_clock::now();
    chrono::duration<double> e2 = et2 - st2;
    cout << FLAGS_queries << "," << dbtime << "," << pftime << "," << checksum << endl;
}

void executeAndPostFilteringRocksDBQueries(QMapLazy <Traits <T>> * qMap) {
    double dbtime = 0;
    double pftime = 0;
    auto st1 = std::chrono::system_clock::now();
    rocksdb::DB* db;
    rocksdb::Options options;
    NumericComparator cmp;
    options.comparator = &cmp;

    rocksdb::Status status = rocksdb::DB::Open(options, "/dev/shm", &db);
    auto et1 = std::chrono::system_clock::now();
    chrono::duration<double> e1 = et1 - st1;
    dbtime += e1.count();
    assert(status.ok());
    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());

    char buffer[100];
    T checksum = 0;
    auto st2 = std::chrono::system_clock::now();
    for(auto itq = qMap->qMap.begin(); itq != qMap->qMap.end(); itq++) {
        Tinterval leaf = itq->first->interval;
        string start = to_string(leaf.min);
        string limit = to_string(leaf.max);
        for (it->Seek(start);
            it->Valid() && stoi(it->key().ToString()) < leaf.max;
            it->Next()
        ) {
            T key = stoll(it->key().ToString());
            T val = stoll(it->value().ToString());

            auto st4 = std::chrono::system_clock::now();
            vector <Tquery *> intersectedQueries;
            for (size_t i = 0; i < itq->second.size(); i++) {
                if(itq->second[i]->interval.intersects(key)) {
                    intersectedQueries.push_back(itq->second[i]);
                }
            }
            for (size_t s = 0; s < intersectedQueries.size(); s++) {
                checksum += val;
            }
            auto et4 = std::chrono::system_clock::now();
            chrono::duration<double> e4 = et4 - st4;
            pftime += e4.count();
        }
    }
    auto et2 = std::chrono::system_clock::now();
    chrono::duration<double> e2 = et2 - st2;
    dbtime = e2.count() - pftime;
    cout << FLAGS_queries << "," << dbtime << "," << pftime << "," << checksum << endl;
}


void executeRocksDBQuery(Tinterval * interval) {
    auto st1 = std::chrono::system_clock::now();
    rocksdb::DB* db;
    rocksdb::Options options;
    NumericComparator cmp;
    options.comparator = &cmp;

    rocksdb::Status status = rocksdb::DB::Open(options, "/dev/shm", &db);
    assert(status.ok());
    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());

    string start = to_string(interval->min);
    string limit = to_string(interval->max);
    for (it->Seek(start);
        it->Valid() && stoi(it->key().ToString()) < interval->max;
        it->Next()
    ) {
        // T key = stoll(it->key().ToString());
        // T value = stoll(it->value().ToString());
    }
    delete db;
}

void postFiltering(vector<Tresult *> table, queryMap & qMap) {
    T sum = 0;
    for (size_t i = 0; i < table.size(); i++) {
        Tquery * query = table[i]->query;
        vector<pair<T, T>> collection = table[i]->collection;
        qArray userQueries = qMap[query];
        for(size_t j = 0; j < collection.size(); j++) {
            qArray intersectedQueries;
            T key = collection[j].first;
            T val = collection[j].second;
            for (size_t k = 0; k < userQueries.size(); k++) {
                if (userQueries[k]->interval.intersects(key)) {
                    intersectedQueries.push_back(userQueries[k]);
                }
            }
            for (size_t k = 0; k < intersectedQueries.size(); k++) {
                sum += val;
            }
        }
    }
    cout << "sum: " << sum << endl;
}


void queryIndexing(vector <Tquery *> queryPlan, T leafSize) {
    QMapLazy <Traits <T>> * qMap = new QMapLazy <Traits <T>>();
    UITree <Traits <T> > * uitree = new UITree <Traits <T> >(leafSize, qMap);
    uitree->insert(queryPlan);
    queryMap qMapPlain = qMap->plain();
    vector<Tquery *> newQueryPlan;
    for(auto it = qMapPlain.begin(); it != qMapPlain.end(); it++) {
        newQueryPlan.push_back(it->first);
    }
    vector<Tresult *> results = executeRocksDBQueries(newQueryPlan);
    postFiltering(results, qMapPlain);
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

    T leafSize = 0;

    T * queriesMeta = getQueriesMetaData(queries);

    long long int checks = getFastQueriesChecksum(queries);

    if (isNumber(FLAGS_leaf_size)) {
        leafSize = atol(FLAGS_leaf_size.c_str());
    } else if (FLAGS_leaf_size == "max_range") {
        leafSize = queriesMeta[2];
    }

    queryIndexing(queries, leafSize);

    // auto st2 = std::chrono::system_clock::now();
    // vector <Tresult*> results = executeRocksDBQueries(queries);
    // auto et2 = std::chrono::system_clock::now();
    // chrono::duration<double> e2 = et2 - st2;
    // cout << "et: " << e2.count() << endl;
    // cout << results.size() << endl;
    // executeRocksDBQueries(queries);
    // auto st1 = std::chrono::system_clock::now();
    // queryIndexing(queries, leafSize);
    // // postFiltering(newQueryPlan);
    // auto et1 = std::chrono::system_clock::now();
    // chrono::duration<double> e1 = et1 - st1;
    // cout << "t1: " << e1.count() << endl;
    // auto result = con.Query("SELECT 42");
    // result->Print();

    gflags::ShutDownCommandLineFlags();

    return 0;
}
