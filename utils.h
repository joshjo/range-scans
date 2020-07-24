#ifndef UTILS_H
#define UTILS_H

#define SEED 100

#include "../interval-base-tree/src/newtree.h"

#include "zipf.h"

using namespace std;

typedef long T;
typedef Interval<T> Tinterval;

T * getQueriesMeta(vector<Tinterval> & queries) {
    T max = queries[0].length();
    T min = queries[0].length();
    T avg = queries[0].length();
    for (int i = 1; i < queries.size(); i++) {
        T l = queries[i].length();
        if (l > max) {
            max = l;
        }
        if (l < min) {
            min = l;
        }
        avg += l;
    }

    T * result = new T [3];
    result[0] = min;
    result[1] = avg / queries.size();
    result[2] = max;

    return result;
}

bool is_number(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(),
        s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

vector<Tinterval> create_queries(T num_queries, T domain_size, T range_size, bool random_range_sizes=false, int percentage_point_queries=0) {
    srand(SEED);
    vector<Tinterval> result;

    for (int i = 0; i < num_queries; i += 1) {
        T rs = range_size;
        if ((rand() % 100) < percentage_point_queries) {
            rs = 1;
        } else if (random_range_sizes) {
            rs = rand() % (domain_size - 2) % range_size;
        }
        T max_random = domain_size - rs;
        T rnd = rand() % max_random;
        result.push_back(Tinterval(rnd, rnd + rs));
    }

    return result;
}

vector<Tinterval> create_queries_zipf(T num_queries, T domain_size, T range_size, bool random_range_sizes=false, int percentage_point_queries=0) {
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
