#include "src/citree.h"
#include "src/aitree.h"

using namespace std;

typedef long long T;

int main() {
    srand(100);

    CITree<T> citree;
    AITree<T> aitree;

    vector<Interval<T>> intervals;

    for(int i = 0; i < 1000; i += 1) {
        // cout << rand() % 100000 << endl;
        long r = rand() % 1000;
        int w = ((rand() % 9) + 1) * 10;
        intervals.push_back(Interval<T>(r, r + w));
    }

    citree.insert(intervals);

    cout << citree.numberOfIntervals() << endl;

    vector<Interval<T> *> vresult = citree.find(450);

    cout << vresult.size() << endl;


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
