#ifndef TOOLS_H
#define TOOLS_H

#include "query.h"
#include "interval.h"
#include "defaultflags.h"

using namespace std;

typedef long long T;
typedef Interval<T> Tinterval;
typedef Query<T> Tquery;


struct CSVresults {
    // qq: Queries information values
    // ii: Indexing values
    // ui: Updatable Interval Tree values
    // db: DB execution values
    // pf: Postfiltering values
    // ff: Flags values

    typedef long I;
    typedef long long int X;
    typedef string S;
    typedef double F;
    I ff_iter;
    S ff_strategy;
    S ff_distribution;
    I ff_num_queries;
    I ff_domain;
    S ff_leaf_len;
    I ff_range_size;
    S ff_database;
    I ff_percentage_point_queries;

    T qq_max_query_value;
    T qq_min_query_value;
    T qq_avg_query_value;

    I ui_avg_node_len;
    I ui_min_node_len;
    I ui_max_node_len;
    I ui_leaf_nodes;
    I ui_tree_depth;

    F pf_time;
    F db_time;
    F ii_time;
    F qm_time;

    X checksum;

    CSVresults() {
        ff_iter = FLAGS_iter;
        ff_strategy = FLAGS_strategy;
        ff_distribution = FLAGS_distribution;
        ff_num_queries = FLAGS_queries;
        ff_domain = FLAGS_domain;
        ff_leaf_len = FLAGS_leaf_size;
        ff_range_size = FLAGS_range_size;
        ff_database = FLAGS_database;
        ff_percentage_point_queries = FLAGS_percentage_point_queries;
    }

    void printFFHeader() {
        if (FLAGS_exec_database) {
            std::cout << "database,";
        }
        if (FLAGS_percentage_point_queries) {
            std::cout << "percentage point queries,";
        }
        std::cout << "iter,strategy,distribution,num queries,domain,leaf len,range size";
    }

    void printFFValues() {
        if (FLAGS_exec_database) {
            std::cout << ff_database << ",";
        }
        if (FLAGS_percentage_point_queries) {
            std::cout << ff_percentage_point_queries << ",";
        }
        std::cout << ff_iter << "," << ff_strategy << "," << ff_distribution << ",";
        std::cout << ff_num_queries << "," << ff_domain << "," << ff_leaf_len << ",";
        std::cout << ff_range_size;
    }

    void printQQHeader() {
        std::cout << "max query value,min query value,avg query value";
    }

    void printQQValues() {
        std::cout << qq_max_query_value << "," << qq_min_query_value << "," << qq_avg_query_value;
    }

    void printUIHeader() {
        std::cout << "avg node len,min node len,max node len,leaf nodes,tree depth";
    }

    void printUIValues() {
        std::cout << ui_avg_node_len << "," << ui_min_node_len << "," << ui_max_node_len << ",";
        std::cout << ui_leaf_nodes << "," << ui_tree_depth;
    }

    void printPFHeader() {
        std::cout << "post filtering time (s)";
    }

    void printPFValues() {
        std::cout << pf_time;
    }

    void printIIHeader() {
        std::cout << "indexing time (s)";
    }

    void printQMHeader() {
        std::cout << "query mapping time (s)";
    }

    void printIIValues() {
        std::cout << ii_time;
    }

    void printQMValues() {
        std::cout << qm_time;
    }

    void printDBHeader() {
        std::cout << "database time (s)";
    }

    void printDBValues() {
        std::cout << db_time;
    }

    void printChecksumValues() {
        std::cout << checksum;
    }

    void printChecksumHeader() {
        std::cout << "checksum";
    }

    void printHeader() {
        printFFHeader();
        std::cout << ",";
        printQQHeader();
        std::cout << ",";
        printUIHeader();
        std::cout << ",";
        printIIHeader();
        std::cout << ",";
        printQMHeader();
        if (FLAGS_exec_database) {
            std::cout << ",";
            printDBHeader();
            if (FLAGS_exec_postfiltering) {
                std::cout << ",";
                printPFHeader();
            }
        }

        if (FLAGS_exec_checksum) {
            std::cout << ",";
            printChecksumHeader();
        }

        std::cout << std::endl;
    }

    void printValues() {
        printFFValues();
        std::cout << ",";
        printQQValues();
        std::cout << ",";
        printUIValues();
        std::cout << ",";
        printIIValues();
        std::cout << ",";
        printQMValues();

        if (FLAGS_exec_database) {
            std::cout << ",";
            printDBValues();
            if (FLAGS_exec_postfiltering) {
                std::cout << ",";
                printPFValues();
            }
        }

        if (FLAGS_exec_checksum) {
            std::cout << ",";
            printChecksumValues();
        }
        std::cout << std::endl;
    }

    void setUIValues(T * data) {
        ui_avg_node_len = data[0];
        ui_min_node_len = data[1];
        ui_max_node_len = data[2];
        ui_leaf_nodes = data[3];
        ui_tree_depth = data[4];
    }

    void setQQValues(T * data) {
        qq_min_query_value = data[0];
        qq_avg_query_value = data[1];
        qq_max_query_value = data[1];
    }
};

vector<Tquery *> create_random_queries() {
    srand(FLAGS_seed);
    vector<Tquery *> result;

    for (int i = 0; i < FLAGS_queries; i += 1) {
        T rs = FLAGS_range_size;
        if ((rand() % 100) < FLAGS_percentage_point_queries) {
            rs = 1;
        } else if (FLAGS_random_range_size) {
            rs = FLAGS_min_range_size + rand() % (FLAGS_max_range_size - FLAGS_min_range_size);
        }
        T max_random = FLAGS_domain - rs;
        T rnd = rand() % max_random;
        result.push_back(new Tquery(Tinterval(rnd, rnd + rs)));
    }

    return result;
}


T * getQueriesMetaData(vector<Tquery *> & queries) {
    T max = queries[0]->interval.length();
    T min = queries[0]->interval.length();
    T avg = queries[0]->interval.length();
    for (int i = 1; i < queries.size(); i++) {
        T l = queries[i]->interval.length();
        if (l > max) {
            max = l;
        }
        if (l < min) {
            min = l;
        }
        avg += l;
    }

    T * result = new T [3];
    result[0] = min;
    result[1] = avg / queries.size();
    result[2] = max;

    return result;
}


bool isNumber(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(),
        s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}


long long int getFastQueriesChecksum(vector<Tquery *> & queries) {
    /*
    WARNING: This function could not return the correct value
    Since in the creation of the tests the values are the double of the key.
    So for instance: 100=200, 450=900.
    */

   long long int checksum = 0;

   for(size_t i=0; i < queries.size(); i++) {
       for(T a = queries[i]->interval.min; a < queries[i]->interval.max; a++) {
           checksum += (a * 2);
       }
   }

   return checksum;
}

#endif // TOOLS_H
