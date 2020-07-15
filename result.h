#ifndef RESULT_H
#define RESULT_H

#include "../interval-base-tree/src/newtree.h"


using namespace std;

struct Result {
    int iter;
    string distribution;
    int number_queries;
    string strategy;
    long leaf_size;
    long range_size;
    long domain;
    double post_filtering_time;
    double db_exec_time;
    double total_time;

    Result(
        int i,
        string dst, int nq, string str,
        long d, long ls, long rs,
        double pft, double det
    ) {
        iter = i;

        distribution = dst;
        number_queries = nq;
        strategy = str;

        domain = d;
        leaf_size = ls;
        range_size = rs;

        post_filtering_time = pft;
        db_exec_time = det;
    }

    template <class T>
    void printCSV(Tree <Traits <T> > * & tree, double tree_total_time, double mapping_time = 0) {
        double mt, ttt, tbt, t2t;
        tbt = tree_total_time - tree->qMap->elapsedTime();

        total_time = tree_total_time + post_filtering_time + db_exec_time;

        T * leafs_values = tree->getLeafsData();
        cout << iter << ",";
        cout << distribution << "," << number_queries << "," << strategy << ",";
        cout << domain << "," << leaf_size << "," << range_size << ",";
        cout << leafs_values[0] << "," << leafs_values[3] << "," << leafs_values[4] << ",";

        cout << tree->qMap->csv() << ",";
        if (mapping_time > 0) {
            t2t = mapping_time;
            mt = 0;
            ttt = tbt + mapping_time;
        } else {
            mt = tree->qMap->elapsedTime();
            t2t = 0;
            ttt = tree_total_time;
        }
        cout << mt << "," << t2t << "," << tbt << "," << ttt << ",";
        cout << post_filtering_time << "," << db_exec_time << ",";
        cout << total_time;
        cout << endl;
    }
};

#endif // RESULT_H
