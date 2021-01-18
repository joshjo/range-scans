#include "src/citree.h"
#include "src/aitree.h"
#include "src/query.h"

using namespace std;

typedef long long T;
typedef Query<T> Tquery;

int main() {
    srand(100);
    vector<Tquery *> user_queries;
    int to_insert = 100000;

    for(int i = 0; i < to_insert; i += 1) {
        long r = rand() % 10000000;
        int w = ((rand() % 9) + 1) * 10000;
        user_queries.push_back(new Tquery(Interval<T>(r, r + w)));
    }

    CITree<T> citree;
    AITree<T> aitree;
    // cout << aitree.size() << endl;
    T val_to_find = 200;

    auto st_ci = chrono::system_clock::now();
    citree.insert(user_queries);
    auto et_ci = chrono::system_clock::now();
    chrono::duration<double> es_ci = et_ci - st_ci;

    auto st_cf = chrono::system_clock::now();
    vector<Tquery *> ci_result = citree.find(val_to_find);
    auto et_cf = chrono::system_clock::now();
    chrono::duration<double> es_cf = et_cf - st_cf;

    cout << "ci insert: " << es_ci.count() << endl;
    cout << "ci find  : " << es_cf.count() << endl;
    cout << "count    : " << ci_result.size() << endl;

    auto st_ai = chrono::system_clock::now();
    for (int i = 0; i < user_queries.size(); i++) {
        aitree.insert(user_queries[i]);
        // if (i > (to_insert - 2)) {
        //     // cout << intervals[i] << endl;
        //     aitree.insert(intervals[i]);
        //     // cout << aitree.graphviz(to_string(i)) << endl << endl << endl;
        // } else {
        //     aitree.insert(intervals[i]);
        // }
    }
    auto et_ai = chrono::system_clock::now();
    chrono::duration<double> es_ai = et_ai - st_ai;

    auto st_af = chrono::system_clock::now();
    vector<Tquery *> ai_result = aitree.find(val_to_find);
    auto et_af = chrono::system_clock::now();

    chrono::duration<double> es_af = et_af - st_af;

    cout << "ai insert: " << es_ai.count() << endl;
    cout << "ai find  : " << es_af.count() << endl;
    cout << "count    : " << ai_result.size() << endl;

    // cout << "------> " << aitree.count() << endl;

    // // cout << node->centers.size() << endl;

    // // cout << "node: " << node-> << endl;


    // cout << "centered interval tree: " << endl;

    // for(int i = 0; i < ci_result.size(); i++) {
    //     cout << ci_result[i]->interval << endl;
    // }

    // cout << "augmented interval tree: " << endl;

    // for(int i = 0; i < ai_result.size(); i++) {
    //     cout << ai_result[i]->interval << endl;
    // }

    // cout << aitree.graphviz() << endl;


    // // intervals.push_back(Interval<long>(1224653905, 1224653925));

    // // Interval<long> i(1224653905, 1224653925);
    // // intervals.push_back(i);

    // // cout << "intersects? " << i.intersects(M) << endl;

    // // cout << M << endl;
    // // intervals.push_back(Interval<long>(807009856, 807009876));
    // // intervals.push_back(Interval<long>(1224653905, 1224653925));

    // // intervals.push_back(Interval<int>(60, 120));

    // // cout << citree.root->centers.size() << endl;
    // // cout << citree.root->left->centers.size() << endl;

    // // cout << citree.graphviz() << endl;
    // // cout << aitree.verifyWeights() << endl;
}
