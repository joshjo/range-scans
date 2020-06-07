#ifndef RESULT_H
#define RESULT_H

#include "../interval-base-tree/src/tree.h"


using namespace std;

struct Result {
    string distribution;
    int number_queries;
    string strategy;
    double number_leafs;
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
        string dst, int nq, string str,
        long iaw, long idw, long itw,
        double tt, double ut, double et,
        double dbt, double rst,
        double t
    ) {
        distribution = dst;
        number_queries = nq;
        strategy = str;
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
        cout << number_queries << "," << strategy << ",";
        cout << interval_avg_width << "," << interval_min_width << "," << interval_max_width << ",";
        cout << tree_time << "," << update_time << "," << extra_time << ",";
        cout << results_separation_time << "," << db_exec_time << ",";
        cout << total_time;
    }
};

#endif // RESULT_H
