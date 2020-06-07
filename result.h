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
    long interval_max_width;
    long interval_min_width;
    long interval_avg_width;
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
        long iaw, long idw, long itw,
        double tt, double ut, double et,
        double dbt, double rst,
        double t
    ) {
        iter = i;

        distribution = dst;
        number_queries = nq;
        strategy = str;

        domain = d;
        leaf_size = ls;
        range_size = rs;

        interval_max_width = itw;
        interval_min_width = idw;
        interval_avg_width = iaw;

        tree_time = tt;
        update_time = ut;
        extra_time = et;

        db_exec_time = dbt;
        results_separation_time = rst;

        total_time = t;
    }

    void printCSV() {
        cout << iter << ",";
        cout << distribution << "," << number_queries << "," << strategy << ",";
        cout << domain << "," << leaf_size << "," << range_size << ",";
        cout << interval_avg_width << "," << interval_min_width << "," << interval_max_width << ",";
        cout << tree_time << "," << update_time << "," << extra_time << ",";
        cout << results_separation_time << "," << db_exec_time << ",";
        cout << total_time;
        cout << endl;
    }
};

#endif // RESULT_H
