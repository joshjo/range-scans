#include "src/uitree.h"

using namespace std;

typedef long long T;
typedef Interval<T> Tinterval;
typedef Query<T> Tquery;


int main() {
    vector<Tquery *> user_queries;
    int to_insert = 1000000;
    T W = 10000;

    for(int i = 0; i < to_insert; i += 1) {
        long r = rand() % 1000000;
        int w = ((rand() % 9) + 1) * W;
        user_queries.push_back(new Tquery(Interval<T>(r, r + w)));
    }

    QMapLazy <Traits <T>> * qMap = new QMapLazy <Traits <T>>();
    UITree<Traits <T> > uitree(W * 10, qMap);

    for (int i = 0; i < user_queries.size(); i++) {
        uitree.insert(user_queries[i]);
    }


    return 0;
}
