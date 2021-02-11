#include "src/tmp.h"

using namespace std;

typedef long long T;
typedef Interval<T> Tinterval;

vector<Tinterval> create_intervals(
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
    vector<Tinterval> result;

    for (int i = 0; i < num_queries; i += 1) {
        T rs = range_size;
        if ((rand() % 100) < percentage_point_queries) {
            rs = 1;
        } else if (random_range_sizes) {
            rs = min_range_size + rand() % (max_range_size - min_range_size);
        }
        T max_random = domain_size - rs;
        T rnd = rand() % max_random;
        result.push_back(Tinterval(rnd, rnd + rs));
    }

    return result;
}


int main() {
    vector<Tinterval> user_queries = create_intervals(1000000, 1000000000, 100000);

    auto st_ai = chrono::system_clock::now();
    QMapLazy <Traits <T>> * qMap = new QMapLazy <Traits <T>>();
    Tree<Traits <T> > uitree(100000, qMap);

    for (int i = 0; i < user_queries.size(); i++) {
        uitree.insert(user_queries[i]);
    }
    auto et_ai = chrono::system_clock::now();
    chrono::duration<double> es_ai = et_ai - st_ai;

    cout << "time: " << es_ai.count() << endl;

    // vector<Tinterval> user_intervals = create_intervals(1000000, 1000000000, 100000);


    return 0;
}
