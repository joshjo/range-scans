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
#include "src/sqlite3pp/headeronly_src/sqlite3pp.h"

using namespace std;

typedef long long T;
typedef Interval<T> Tinterval;
typedef Query<T> Tquery;


int noopCallback(void *NotUsed, int argc, char **argv, char **azColName){
    return 0;
}

int callbackSelect(void *NotUsed, int argc, char **argv, char **azColName){

    // int argc: holds the number of results
    // (array) azColName: holds each column returned
    // (array) argv: holds each value

    // cout << "# rows " << argc << endl;

    // for(int i = 0; i < argc; i++) {

    //     // Show column name, value, and newline
    //     cout << azColName[i] << ": " << argv[i] << endl;

    // }

    // Insert a newline
    // cout << endl;

    // Return successful
    return 0;
}

long qat3(vector <Tquery *> & queryPlan) {
    sqlite3pp::database db("/dev/shm/sqlite_simple.db");

    char buffer[100];
    T checksum = 0;
    for(size_t i = 0; i < queryPlan.size(); i++) {
        Tinterval leaf = queryPlan[i]->interval;
        sprintf(buffer, "SELECT * FROM simple WHERE key >= %lld AND key < %lld", leaf.min, leaf.max);
        sqlite3pp::query qry(db, buffer);
        T key, value;

        for (sqlite3pp::query::iterator i = qry.begin(); i != qry.end(); ++i) {
            std::tie(key, value) = (*i).get_columns<T, T>(0, 1);
        }
    }

    return 0;
}

long qat(sqlite3 * db, vector <Tquery *> & queryPlan) {

    char buffer[100];
    char *zErrMsg = 0;

    for(size_t i = 0; i < queryPlan.size(); i += 1) {
        sprintf(
            buffer,
            "SELECT * FROM simple WHERE KEY >= %lld AND KEY < %lld;",
            queryPlan[i]->interval.min,
            queryPlan[i]->interval.max
        );
        int rc = sqlite3_exec(db, buffer, noopCallback, 0, &zErrMsg);
    }

    return 0;
}

int main(int argc, char * argv[]) {
    srand (FLAGS_seed);

    gflags::SetUsageMessage("Main");
    gflags::SetVersionString("1.0.0");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    vector <Tquery *> queries;

    queries = create_random_queries();

    sqlite3 * db;
    // int rc = sqlite3_open(":memory:", &db);
    int rc = sqlite3_open("/dev/shm/sqlite_test.db", &db);
    string sql;
    char *zErrMsg = 0;
    if(rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return(0);
    } else {
        fprintf(stderr, "Opened database successfully\n");
    }

    sql = "CREATE TABLE simple ("  \
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

    // char buffer[100];

    // for (int i = 1; i <= 1000000; i++) {
    //     // Save SQL insert data
    //     // string key = to_string(i);
    //     // string value = to_string(i * 2);
    //     // sql = ;
    //     sprintf(buffer, "INSERT INTO simple ('KEY', 'VALUE') VALUES ('%d', '%d');", i, i * 2);
    //     rc = sqlite3_exec(db, buffer, noopCallback, 0, &zErrMsg);
    //     // format("{} {}!", "Hello", "world", "something");
    // }

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
    // qat(db, queries);
    qat3(queries);
    auto et2 = chrono::system_clock::now();

    chrono::duration<double> es2 = et2 - st2;
    cout << "time: " << es2.count();

    // // Run the SQ    L (convert the string to a C-String with c_str() )


    sqlite3_close(db);

    return 0;
}
