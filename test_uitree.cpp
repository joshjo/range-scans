#include "src/uitree.h"
#include "src/tools.h"

using namespace std;

typedef long long T;
typedef Interval<T> Tinterval;
typedef Query<T> Tquery;


int main() {
    vector<Tquery *> user_queries = create_random_queries(1000000, 1000000000, 100000);

    auto st_ai = chrono::system_clock::now();
    QMapEager <Traits <T>> * qMap = new QMapEager <Traits <T>>();
    UITree<Traits <T> > uitree(100000, qMap);

    for (int i = 0; i < user_queries.size(); i++) {
        uitree.insert(user_queries[i]);
    }
    auto et_ai = chrono::system_clock::now();
    chrono::duration<double> es_ai = et_ai - st_ai;

    cout << "time: " << es_ai.count() << endl;

    // vector<Tinterval> user_intervals = create_intervals(1000000, 1000000000, 100000);


    return 0;
}
