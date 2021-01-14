#include "src/citree.h"
#include "src/aitree.h"

using namespace std;

typedef long long T;

int main() {
    srand(100);

    CITree<T> citree;
    AITree<T> aitree;

    vector<Interval<T>> intervals;

    T val_to_find = 9087;

    for(int i = 0; i < 100000; i += 1) {
        long r = rand() % 1000000;
        int w = ((rand() % 9) + 1) * 1000;
        intervals.push_back(Interval<T>(r, r + w));
    }

    auto st_ci = chrono::system_clock::now();
    citree.insert(intervals);
    auto et_ci = chrono::system_clock::now();
    chrono::duration<double> es_ci = et_ci - st_ci;

    cout << citree.numberOfIntervals() << endl;

    auto st_cf = chrono::system_clock::now();
    vector<Interval<T> *> ci_result = citree.find(val_to_find);
    auto et_cf = chrono::system_clock::now();
    chrono::duration<double> es_cf = et_cf - st_cf;

    cout << "ci insert: " << es_ci.count() << endl;
    cout << "ci find  : " << es_cf.count() << endl;
    cout << "count    : " << ci_result.size() << endl;

    auto st_ai = chrono::system_clock::now();
    for (int i = 0; i < intervals.size(); i++) {
        aitree.insert(intervals[i]);
    }
    auto et_ai = chrono::system_clock::now();
    chrono::duration<double> es_ai = et_ai - st_ai;

    auto st_af = chrono::system_clock::now();
    vector<Interval<T> *> ai_result = aitree.find(val_to_find);
    auto et_af = chrono::system_clock::now();

    chrono::duration<double> es_af = et_af - st_af;

    cout << "ai insert: " << es_ai.count() << endl;
    cout << "ai find  : " << es_af.count() << endl;
    cout << "count    : " << ai_result.size() << endl;

    // cout << node->centers.size() << endl;

    // cout << "node: " << node-> << endl;


    // for(int i = 0; i < ci_result.size(); i++) {
    //     cout << *(ci_result[i]) << endl;
    // }


    // intervals.push_back(Interval<long>(1224653905, 1224653925));

    // Interval<long> i(1224653905, 1224653925);
    // intervals.push_back(i);

    // cout << "intersects? " << i.intersects(M) << endl;

    // cout << M << endl;
    // intervals.push_back(Interval<long>(807009856, 807009876));
    // intervals.push_back(Interval<long>(1224653905, 1224653925));

    // intervals.push_back(Interval<int>(60, 120));

    // cout << citree.root->centers.size() << endl;
    // cout << citree.root->left->centers.size() << endl;

    // cout << citree.graphviz() << endl;
}
