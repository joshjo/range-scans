#include <assert.h>
#include <iostream>
#include <cmath>
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "src/sqlite3pp/headeronly_src/sqlite3pp.h"
#include "duckdb.hpp"
#include "numeric_comparator.h"


using namespace std;
using namespace duckdb;

typedef long long T;


void writhInSQLite3Blocks(T start, T limit) {
    // sqlite3pp::database db(":memory:");
    sqlite3pp::database db("/dev/shm/duck_sqlite3.db");

    // DuckDB db("/dev/shm/duck_simple.db");
    int blockSize = 10000;
    float numBlocksF = (float) (limit - start) / blockSize;
    int numBlocks = ceil(numBlocksF);
    db.execute("DROP TABLE IF EXISTS simple");
    db.execute("CREATE TABLE simple(key INTEGER PRIMARY KEY , value INTEGER)");
    // sqlite3pp::query qryi(db, "CREATE INDEX simple_key ON simple(key);");
    auto st2 = std::chrono::system_clock::now();

    for (int x = 0; x < numBlocks; x++) {
        string buffer = "INSERT INTO simple VALUES ";
        T blockStart = start + (x * blockSize);
        T blockLimit = blockStart + blockSize - 1;
        blockLimit = blockLimit > limit ? limit : blockLimit;
        for (T i = blockStart; i <= blockLimit; i += 1) {
            buffer += "(" + to_string(i) + ", " + to_string(i * 2) + ")";
            if (i < blockLimit) {
                buffer += ", ";
            }
        }
        db.execute(buffer.c_str());
    }
    auto et2 = std::chrono::system_clock::now();
    chrono::duration<double> e2 = et2 - st2;
    cout << "ti: " << e2.count() << endl;

    sqlite3pp::query qry(db, "SELECT COUNT(*) FROM simple");
    for (int i = 0; i < qry.column_count(); ++i) {
        cout << qry.column_name(i) << "\t";
    }

    for (sqlite3pp::query::iterator i = qry.begin(); i != qry.end(); ++i) {
        for (int j = 0; j < qry.column_count(); ++j) {
            cout << (*i).get<char const*>(j) << "\t";
        }
        cout << endl;
    }

    sqlite3pp::query qry2(db, "SELECT sql FROM sqlite_master WHERE name = 'simple';");
    for (int i = 0; i < qry2.column_count(); ++i) {
        cout << qry2.column_name(i) << "\t";
    }

    for (sqlite3pp::query::iterator i = qry2.begin(); i != qry2.end(); ++i) {
        for (int j = 0; j < qry2.column_count(); ++j) {
            cout << (*i).get<char const*>(j) << "\t";
        }
        cout << endl;
    }
}

void writhInSQLite3(T start, T limit) {
    // sqlite3pp::database db(":memory:");
    sqlite3pp::database db("/dev/shm/sqlite3_simple.db");

    // DuckDB db("/dev/shm/duck_simple.db");
    int blockSize = 10000;
    db.execute("DROP TABLE IF EXISTS simple");
    db.execute("CREATE TABLE simple(key INTEGER PRIMARY KEY , value INTEGER)");
    // sqlite3pp::query qryi(db, "CREATE INDEX simple_key ON simple(key);");
    auto st2 = std::chrono::system_clock::now();

    for (T i = start; i <= limit; i += 1) {
        char buffer[100];
        sprintf(buffer, "INSERT INTO simple ('KEY', 'VALUE') VALUES ('%lld', '%lld');", i, i * 2);
        db.execute(buffer);
    }
    auto et2 = std::chrono::system_clock::now();
    chrono::duration<double> e2 = et2 - st2;
    cout << "ti: " << e2.count() << endl;

    sqlite3pp::query qry(db, "SELECT COUNT(*) FROM simple");
    for (int i = 0; i < qry.column_count(); ++i) {
        cout << qry.column_name(i) << "\t";
    }

    for (sqlite3pp::query::iterator i = qry.begin(); i != qry.end(); ++i) {
        for (int j = 0; j < qry.column_count(); ++j) {
            cout << (*i).get<char const*>(j) << "\t";
        }
        cout << endl;
    }

    sqlite3pp::query qry2(db, "SELECT sql FROM sqlite_master WHERE name = 'simple';");
    for (int i = 0; i < qry2.column_count(); ++i) {
        cout << qry2.column_name(i) << "\t";
    }

    for (sqlite3pp::query::iterator i = qry2.begin(); i != qry2.end(); ++i) {
        for (int j = 0; j < qry2.column_count(); ++j) {
            cout << (*i).get<char const*>(j) << "\t";
        }
        cout << endl;
    }
}


void writeInRocks(T start, T limit) {
    rocksdb::DB* db;
    rocksdb::Options options;
    options.create_if_missing = true;
    NumericComparator cmp;
    options.comparator = &cmp;

    rocksdb::Status status = rocksdb::DB::Open(options, "/dev/shm", &db);
    assert(status.ok());

    for (T i = start; i <= limit; i += 1) {
        std::string key = std::to_string(i);
        std::string value = std::to_string(i * 2);
        db->Put(rocksdb::WriteOptions(), key, value);
    }

    delete db;
}

void writeInDuck(T start, T limit) {
    // DuckDB db("josue.db");
    DuckDB db("/dev/shm/duck_simple.db");
    int blockSize = 10000;
    float numBlocksF = (float) (limit - start) / blockSize;
    int numBlocks = ceil(numBlocksF);
	Connection con(db);
    con.Query("DROP TABLE IF EXISTS simple");
	con.Query("CREATE TABLE simple(key INTEGER, value INTEGER)");
    auto st2 = std::chrono::system_clock::now();

    for (int x = 0; x < numBlocks; x++) {
        string buffer = "INSERT INTO simple VALUES ";
        T blockStart = start + (x * blockSize);
        T blockLimit = blockStart + blockSize - 1;
        blockLimit = blockLimit > limit ? limit : blockLimit;
        for (T i = blockStart; i <= blockLimit; i += 1) {
            buffer += "(" + to_string(i) + ", " + to_string(i * 2) + ")";
            if (i < blockLimit) {
                buffer += ", ";
            }
        }
        con.Query(buffer);
    }
    auto et2 = std::chrono::system_clock::now();
    chrono::duration<double> e2 = et2 - st2;
    cout << "ti: " << e2.count() << endl;

    auto result = con.Query("SELECT COUNT(*) FROM simple");
    result->Print();
    auto resultDescribe = con.Query("DESCRIBE simple");
    resultDescribe->Print();
}


int main(int argc, char** argv) {
    if (argc != 3) {
        cout << "Please enter a range: ./simple 100 1000" << endl;
        return 0;
    }

    T start, limit;

    sscanf(argv[1], "%lld", &start);
    sscanf(argv[2], "%lld", &limit);

    writhInSQLite3(start, limit);
    // writeInDuck(start, limit);
    // writeInRocks(start, limit);
}

