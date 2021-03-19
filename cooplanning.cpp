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
#include "src/defaultflags.h"
#include "duckdb.hpp"

typedef long long T;
typedef Interval<T> Tinterval;
typedef Query<T> Tquery;
typedef vector <Tquery *> qArray;
typedef Result <T> Tresult;
typedef unordered_map<Tquery *, qArray> queryMap;

using namespace std;


T leafSize = 0;

CSVresults * csvResults;

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

vector<Tresult *> executeDuckDBQueries(vector <Tquery *> & queryPlan) {
    vector <Tresult *> table;
    duckdb::DuckDB db("/dev/shm/josue.db");
	duckdb::Connection con(db);
    char buffer[100];
    T checksum = 0;
    for(size_t i = 0; i < queryPlan.size(); i++) {
        Tresult * row = new Tresult(queryPlan[i]);
        Tinterval leaf = queryPlan[i]->interval;
        sprintf(buffer, "SELECT * FROM simple WHERE key >= %lld AND key < %lld", leaf.min, leaf.max);
        auto result = con.Query(buffer);
        for (size_t r = 0; r < result->collection.Count(); r++) {
            T val = result->GetValue<int64_t>(1, r);
            T key = result->GetValue<int64_t>(0, r);
            row->appendResult(key, val);
        }
        table.push_back(row);
    }

    return table;
}

T postFiltering(vector<Tresult *> table, queryMap & qMap) {
    T sum = 0;
    double tt1 = 0;
    double tt2 = 0;
    double tt3 = 0;

    auto st1 = std::chrono::system_clock::now();
    for (size_t i = 0; i < table.size(); i++) {
        Tquery * query = table[i]->query;
        vector<pair<T, T>> collection = table[i]->collection;
        qArray userQueries = qMap[query];
        auto st2 = std::chrono::system_clock::now();
        for(size_t j = 0; j < collection.size(); j++) {
            qArray intersectedQueries;
            T key = collection[j].first;
            T val = collection[j].second;
            auto st3 = std::chrono::system_clock::now();
            for (size_t k = 0; k < userQueries.size(); k++) {
                if (userQueries[k]->interval.intersects(key)) {
                    intersectedQueries.push_back(userQueries[k]);
                }
            }
            auto et3 = std::chrono::system_clock::now();
            for (size_t k = 0; k < intersectedQueries.size(); k++) {
                sum += val;
            }
            chrono::duration<double> e3 = et3 - st3;
            tt3 += e3.count();
        }
        auto et2 = std::chrono::system_clock::now();
        chrono::duration<double> e2 = et2 - st2;
        tt2 += e2.count();
    }
    auto et1 = std::chrono::system_clock::now();
    chrono::duration<double> e1 = et1 - st1;
    return sum;
}


queryMap queryIndexing(vector <Tquery *> queryPlan){
    auto sti = std::chrono::system_clock::now();
    QMapLazy <Traits <T>> * qMap = new QMapLazy <Traits <T>>();
    UITree <Traits <T> > * uitree = new UITree <Traits <T> >(leafSize, qMap);
    uitree->insert(queryPlan);
    auto eti = std::chrono::system_clock::now();
    chrono::duration<double> ei = eti - sti;
    csvResults->ii_time = ei.count();
    csvResults->setUIValues(uitree->getLeafsData());
    return qMap->plain();
}


void queryCoplanning(vector <Tquery *> queryPlan) {
    // indexing
    queryMap qMapPlain = queryIndexing(queryPlan);
    vector<Tquery *> newQueryPlan;
    for(auto it = qMapPlain.begin(); it != qMapPlain.end(); it++) {
        newQueryPlan.push_back(it->first);
    }
    // end indexing
    // Execute new query plan
    if (FLAGS_exec_database) {
        auto std = std::chrono::system_clock::now();
        vector<Tresult *> results;
        if (FLAGS_database == "rocksdb") {
            results = executeRocksDBQueries(newQueryPlan);
        } else if (FLAGS_database == "duckdb") {
            results = executeDuckDBQueries(newQueryPlan);
        }
        auto etd = std::chrono::system_clock::now();
        chrono::duration<double> ed = etd - std;
        csvResults->db_time = ed.count();
        if (FLAGS_exec_postfiltering) {
            // Postfiltering
            auto stp = std::chrono::system_clock::now();
            T checksum = postFiltering(results, qMapPlain);
            auto etp = std::chrono::system_clock::now();
            chrono::duration<double> ep = etp - stp;
            csvResults->pf_time = ep.count();
        }
    }

}


int main(int argc, char** argv) {
    srand (FLAGS_seed);
    gflags::SetUsageMessage("Main");
    gflags::SetVersionString("1.0.0");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    vector <Tquery *> queries;

    queries = create_random_queries();
    csvResults = new CSVresults();

    T * queriesMeta = getQueriesMetaData(queries);


    if (FLAGS_exec_checksum) {
        long long int checks = getFastQueriesChecksum(queries);
        csvResults->checksum = checks;
    }

    if (isNumber(FLAGS_leaf_size)) {
        leafSize = atol(FLAGS_leaf_size.c_str());
    } else if (FLAGS_leaf_size == "max_range") {
        leafSize = queriesMeta[2];
    }
    queryCoplanning(queries);

    csvResults->setQQValues(queriesMeta);
    csvResults->printValues();

    gflags::ShutDownCommandLineFlags();

    return 0;
}
