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
    long * leafs_values;
    double tree_building_time;
    double query_indexing_time;
    double post_filtering_time;
    double db_exec_time;
    double total_time;

    Result(
        int i,
        string dst, int nq, string str,
        long d, long ls, long rs,
        long * lv,
        double tbt, double qit,
        double pft, double det
    ) {
        iter = i;

        distribution = dst;
        number_queries = nq;
        strategy = str;

        domain = d;
        leaf_size = ls;
        range_size = rs;

        leafs_values = lv;

        tree_building_time = tbt;
        query_indexing_time = qit;

        post_filtering_time = pft;
        db_exec_time = det;

        total_time = tbt + qit + pft + det;
    }

    void printCSV() {
        cout << iter << ",";
        cout << distribution << "," << number_queries << "," << strategy << ",";
        cout << domain << "," << leaf_size << "," << range_size << ",";
        cout << leafs_values[0] << "," << leafs_values[3] << "," << leafs_values[4] << ",";
        cout << tree_building_time << "," << query_indexing_time << ",";
        cout << post_filtering_time << "," << db_exec_time << ",";
        cout << total_time;
        cout << endl;
    }
};

#endif // RESULT_H
