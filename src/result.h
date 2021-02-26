#ifndef RESULT_H
#define RESULT_H

#include "includes.h"
#include "query.h"


template <class T>
struct Result {
    Query<T> * query;
    vector <pair<T, T>>collection;

    Result(Query<T> * q) : query(q) {}

    void appendResult(T & key, T & value){
        collection.push_back(make_pair(key, value));
    }
};

#endif // RESULT_H
