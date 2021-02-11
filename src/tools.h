#ifndef TOOLS_H
#define TOOLS_H

#include "query.h"
#include "interval.h"

using namespace std;

typedef long long T;
typedef Interval<T> Tinterval;
typedef Query<T> Tquery;


vector<Tquery *> create_random_queries(
        T num_queries
        , T domain_size
        , T range_size
        , bool random_range_sizes=false
        , int min_range_size=0
        , int max_range_size=0
        , int percentage_point_queries=0
        , int seed=100
    ) {
    srand(seed);
    vector<Tquery *> result;

    for (int i = 0; i < num_queries; i += 1) {
        T rs = range_size;
        if ((rand() % 100) < percentage_point_queries) {
            rs = 1;
        } else if (random_range_sizes) {
            rs = min_range_size + rand() % (max_range_size - min_range_size);
        }
        T max_random = domain_size - rs;
        T rnd = rand() % max_random;
        result.push_back(new Tquery(Tinterval(rnd, rnd + rs)));
    }

    return result;
}


T * getQueriesMetaData(vector<Tquery *> & queries) {
    T max = queries[0]->interval.length();
    T min = queries[0]->interval.length();
    T avg = queries[0]->interval.length();
    for (int i = 1; i < queries.size(); i++) {
        T l = queries[i]->interval.length();
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


bool isNumber(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(),
        s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}


long long int getFastQueriesChecksum(vector<Tquery *> & queries) {
    /*
    WARNING: This function could not return the correct value
    Since in the creation of the tests the values are the double of the key.
    So for instance: 100=200, 450=900.
    */

   long long int checksum = 0;

   for(size_t i=0; i < queries.size(); i++) {
       for(T a = queries[i]->interval.min; a < queries[i]->interval.max; a++) {
           checksum += (a * 2);
       }
   }

   return checksum;
}

#endif // TOOLS_H
