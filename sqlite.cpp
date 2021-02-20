#include <iostream>
#include <string>
#include <stdio.h>
#include <sqlite3.h>
#include <gflags/gflags.h>
#include "duckdb.hpp"

#include "src/query.h"
#include "src/citree.h"
#include "src/aitree.h"
#include "src/uitree.h"
#include "src/tools.h"

using namespace std;

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



int noopCallback(void *NotUsed, int argc, char **argv, char **azColName){
    return 0;
}

int callbackSelect(void *NotUsed, int argc, char **argv, char **azColName){

    // int argc: holds the number of results
    // (array) azColName: holds each column returned
    // (array) argv: holds each value

    // cout << "# rows " << argc << endl;

    for(int i = 0; i < argc; i++) {

        // Show column name, value, and newline
        cout << azColName[i] << ": " << argv[i] << endl;

    }

    // Insert a newline
    // cout << endl;

    // Return successful
    return 0;
}

long lazy(sqlite3 * db, vector <Tquery *> & queries, T leaf_size) {
    long sum = 0;

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

    char buffer[100];

    auto st_2 = std::chrono::system_clock::now();
    char *zErrMsg = 0;
    for (auto itq = qMap->qMap.begin(); itq != qMap->qMap.end(); itq++) {

        auto st_3 = std::chrono::system_clock::now();
        CITree <T> qtree;
        qtree.insert(itq->second);
        auto et_3 = chrono::system_clock::now();
        chrono::duration<double> e3 = et_3 - st_3;
        pf_tree_building += e3.count();

        // cout << qtree.graphviz() << endl << endl;

        Tinterval leaf = itq->first->interval;

        sprintf(buffer, "SELECT * FROM 'SIMPLE' WHERE KEY >= %lld AND KEY < %lld;", leaf.min, leaf.max);
        int rc = sqlite3_exec(db, buffer, noopCallback, 0, &zErrMsg);
    }
    auto et_2 = std::chrono::system_clock::now();
    std::chrono::duration<double> e2 = et_2 - st_2;
    db_and_postfiltering = e2.count();

    // post_filtering_time = pf_tree_building + pf_tree_find + query_assignment;
    // db_exec_time = db_and_postfiltering - post_filtering_time;
    // cout << post_filtering_time << "," << pf_tree_building << "," << pf_tree_find << ",";
    // cout << query_assignment << "," << db_exec_time << ",";
    // cout << sum;
    // cout << endl;

    return sum;
}

int main(int argc, char * argv[]) {
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

    sqlite3 * db;
    int rc = sqlite3_open(":memory:", &db);
    string sql;
    char *zErrMsg = 0;
    if(rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return(0);
    } else {
        fprintf(stderr, "Opened database successfully\n");
    }

    sql = "CREATE TABLE SIMPLE ("  \
            "KEY UNSIGNED INT PRIMARY KEY NOT NULL," \
            "VALUE UNSIGNED INT NOT NULL);";

    // Run the SQL (convert the string to a C-String with c_str() )
    rc = sqlite3_exec(db, sql.c_str(), noopCallback, 0, &zErrMsg);

    // if(rc) {
    //     fprintf(stderr, "Can't create table: %s\n", sqlite3_errmsg(db));
    //     return(0);
    // } else {
    //     fprintf(stderr, "Table created successfully\n");
    // }

    char buffer[100];

    for (int i = 1; i <= 1000000; i++) {
        // Save SQL insert data
        // string key = to_string(i);
        // string value = to_string(i * 2);
        // sql = ;
        sprintf(buffer, "INSERT INTO SIMPLE ('KEY', 'VALUE') VALUES ('%d', '%d');", i, i * 2);
        rc = sqlite3_exec(db, buffer, noopCallback, 0, &zErrMsg);
        // format("{} {}!", "Hello", "world", "something");
    }

    // // Run the SQL (convert the string to a C-String with c_str() )
    // rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);

    // // Save SQL insert data
    // sql = "SELECT COUNT(*) FROM 'SIMPLE';";

    // // Run the SQL (convert the string to a C-String with c_str() )
    // rc = sqlite3_exec(db, sql.c_str(), callbackSelect, 0, &zErrMsg);

    // Find queries

    // auto st_1 = chrono::system_clock::now();
    // for (int i = 0; i < queries.size(); i++) {
    //     Tquery * q = queries[i];
    //     T min = q->interval.min;
    //     T max = q->interval.max;
    //     sprintf(buffer, "SELECT * FROM 'SIMPLE' WHERE KEY >= %lld AND KEY < %lld;", min, max);
    //     rc = sqlite3_exec(db, buffer, noopCallback, 0, &zErrMsg);

    //     if(rc) {
    //         fprintf(stderr, "Can't execute query: %s\n", sqlite3_errmsg(db));
    //     }
    // }
    // auto et_1 = chrono::system_clock::now();

    auto st2 = chrono::system_clock::now();
    lazy(db, queries, 100000);
    auto et2 = chrono::system_clock::now();

    chrono::duration<double> es2 = et2 - st2;
    cout << "time: " << es2.count();

    // // Run the SQ    L (convert the string to a C-String with c_str() )


    sqlite3_close(db);

    return 0;
}
