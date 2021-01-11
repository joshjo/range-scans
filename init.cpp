#include "src/citree.h"

using namespace std;


int main() {
    srand(100);

    CITree<long> citree;

    vector<Interval<long>> intervals;

    // for(int i = 0; i < 4; i += 1) {
    //     // cout << rand() % 100000 << endl;
    //     long r = rand() % 100000000000;
    //     int w = ((rand() % 9) + 1) * 10;
    //     intervals.push_back(Interval<long>(r, r + w));
    //     cout << "i: " << intervals[i] << endl;
    // }


    intervals.push_back(Interval<long>(1224653905, 1224653925));

    // Interval<long> i(1224653905, 1224653925);
    // intervals.push_back(i);

    vector<long> ee = getEndpoints(intervals);

    cout << "ee size: " << ee[0] << " - " << ee[1] << endl;

    long M = getMedian(ee);

    cout << "M: " << M << endl;

    // cout << "intersects? " << i.intersects(M) << endl;

    // cout << M << endl;
    // intervals.push_back(Interval<long>(807009856, 807009876));
    // intervals.push_back(Interval<long>(1224653905, 1224653925));

    // citree.insert(intervals);
    // intervals.push_back(Interval<int>(60, 120));

    // cout << citree.root->centers.size() << endl;
    // cout << citree.root->left->centers.size() << endl;

    // cout << citree.graphviz() << endl;
}
