#ifndef RESULT_H
#define RESULT_H

#include "../interval-base-tree/src/tree.h"


using namespace std;

struct Result {
    int iter;
    string distribution;
    int number_queries;
    string strategy;
    long leaf_size;
    long range_size;
    long domain;
    long unsigned * leafs_values;
    double tree_time;
    double update_time;
    double extra_time;
    double db_exec_time;
    double results_separation_time;
    double total_time;

    Result(
        int i,
        string dst, int nq, string str,
        long d, long ls, long rs,
        long unsigned * lv,
        double tt, double ut, double et,
        double rst, double dbt,
        double t
    ) {
        iter = i;

        distribution = dst;
        number_queries = nq;
        strategy = str;

        domain = d;
        leaf_size = ls;
        range_size = rs;

        leafs_values = lv;

        tree_time = tt;
        update_time = ut;
        extra_time = et;

        results_separation_time = rst;
        db_exec_time = dbt;

        total_time = t;
    }

    void printCSV() {
        cout << iter << ",";
        cout << distribution << "," << number_queries << "," << strategy << ",";
        cout << domain << "," << leaf_size << "," << range_size << ",";
        cout << leafs_values[0] << "," << leafs_values[1] << "," << leafs_values[2] << "," << leafs_values[3] << ",";
        cout << tree_time << "," << update_time << "," << extra_time << ",";
        cout << results_separation_time << "," << db_exec_time << ",";
        cout << total_time;
        cout << endl;
    }
};

#endif // RESULT_H
