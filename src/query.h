#ifndef QUERY_H
#define QUERY_H

#include "includes.h"
#include "interval.h"
#include "uuid.h"


template <class T>
class Query {
    typedef Interval<T> Tinterval;

    public:

    string id;
    Tinterval interval;

    Query(Tinterval interval) {
        this->interval = interval;
        this->id = uuid::generate_uuid_v4();
    }
};

#endif // QUERY_H
