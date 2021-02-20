#include <assert.h>
#include <iostream>
#include <cmath>
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "duckdb.hpp"
#include "numeric_comparator.h"


using namespace std;
using namespace duckdb;

typedef long long T;


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
    DuckDB db("/dev/shm/josue.db");
    int blockSize = 10000;
    float numBlocksF = (float) (limit - start) / blockSize;
    int numBlocks = ceil(numBlocksF);
	Connection con(db);
	con.Query("CREATE TABLE simple(key INTEGER PRIMARY KEY, value INTEGER)");
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
}


int main(int argc, char** argv) {
    if (argc != 3) {
        cout << "Please enter a range: ./simple 100 1000" << endl;
        return 0;
    }

    T start, limit;

    sscanf(argv[1], "%lld", &start);
    sscanf(argv[2], "%lld", &limit);

    writeInDuck(start, limit);
}

