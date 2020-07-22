#ifndef UTILS_H
#define UTILS_H

#define SEED 100

#include "../interval-base-tree/src/newtree.h"

#include "zipf.h"

using namespace std;

typedef long T;
typedef Interval<T> Tinterval;

vector<Tinterval> create_queries(T num_queries, T domain_size, T range_size, bool random_range_sizes=false) {
    srand(SEED);

    vector<Tinterval> result;
    T rs = range_size;

    if (random_range_sizes) {
        rs = rand() % (domain_size - 3);
    }

    T max_random = domain_size - rs;
    for (int i = 0; i < num_queries; i += 1) {
        T rnd = rand() % max_random;
        result.push_back(Tinterval(rnd, rnd + rs));
    }
    return result;
}

vector<Tinterval> create_queries_zipf(T num_queries, T domain_size, T range_size, bool random_range_sizes=false) {
    vector<Tinterval> result;
    T max_random = domain_size - range_size;
    std::mt19937 gen(SEED);
    zipf_distribution<> zipf(max_random);

    for (int i = 0; i < num_queries; i += 1) {
        T rnd = zipf(gen);
        result.push_back(Tinterval(rnd, rnd + range_size));
    }
    return result;
}

#endif // UTILS_H
