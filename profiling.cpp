#include <gflags/gflags.h>

#include "../interval-base-tree/src/newtree.h"

#include "result.h"
#include "utils.h"

DEFINE_int64(queries, 100, "Number of queries");
DEFINE_int64(key_domain_size, 1000000, "Key domain size");
DEFINE_string(leaf_size, "100000", "Leaf size");
DEFINE_int64(range_size, 100000, "Range of queries");
DEFINE_bool(random_range_size, false, "Use random range of queries");
DEFINE_int64(percentage_point_queries, 0, "% of Pointe Queries");
DEFINE_string(strategy, "raw", "Strategy");
DEFINE_string(distribution, "normal", "Random Distribution");
DEFINE_int64(iter, 0, "Define the iteration number");
DEFINE_int64(seed, 100, "Random Seed");


void printTimes(T * queriesMeta, Tree <Traits <T> > * & tree, double total_time, double mapping_time = 0) {
    double tbt, mt, t2t, ttt;
    tbt = total_time - tree->qMap->elapsedTime();
    T * leafsData = tree->getLeafsData();

    cout << FLAGS_iter << ",";
    cout << FLAGS_strategy << "," << FLAGS_distribution << "," << FLAGS_queries << ",";
    cout << FLAGS_key_domain_size << "," << FLAGS_leaf_size << "," << FLAGS_range_size << ",";
    cout << queriesMeta[0] << "," << queriesMeta[1] << "," << queriesMeta[2] << ",";
    cout << leafsData[0] << "," << leafsData[3] << "," << leafsData[4] << ",";
    cout << tree->qMap->csv() << ",";
    if (mapping_time > 0) {
        t2t = mapping_time;
        mt = 0;
        ttt = tbt + mapping_time;
    } else {
        t2t = 0;
        mt = tree->qMap->elapsedTime();
        ttt = total_time;
    }
    cout << tbt << "," << mt << "," << t2t << "," << ttt;

    cout << endl;
}

void additional(vector <Tinterval> & queries, T leaf_size, T * queriesMeta) {
    auto start_time = std::chrono::system_clock::now();
    QMapExtra <Traits <T>> * qMap = new QMapExtra <Traits <T>>();
    Tree <Traits <T> > * tree = new Tree <Traits <T> >(leaf_size, qMap);

    for (int i = 0; i < queries.size(); i += 1) {
        tree->insert(queries[i]);
    }

    auto end_time = std::chrono::system_clock::now();
    chrono::duration<double> elapsed_seconds = end_time - start_time;
    double total_time = elapsed_seconds.count();

    start_time = std::chrono::system_clock::now();
    LeafTree<Traits <T>> leaftree;
    vector<Node<T> *> leafs;
    tree->root->getLeafs(leafs);
    random_shuffle(leafs.begin(), leafs.end());

    for (int i = 0; i < leafs.size(); i++) {
        leaftree.insert(leafs[i]->interval);
    }

    for (int i = 0; i < queries.size(); i += 1) {
        leaftree.assign(&queries[i]);
    }
    tree->qMap->indexed = leaftree.numIndexedQueries();

    end_time = std::chrono::system_clock::now();
    elapsed_seconds = end_time - start_time;
    double mapping_time = elapsed_seconds.count();

    printTimes(queriesMeta, tree, total_time, mapping_time);
}

void eager(vector <Tinterval> & queries, T leaf_size, T * queriesMeta) {
    auto start_time = std::chrono::system_clock::now();
    QMapEager <Traits <T>> * qMap = new QMapEager <Traits <T>>();
    Tree <Traits <T> > * tree = new Tree <Traits <T> >(leaf_size, qMap);

    for (int i = 0; i < queries.size(); i += 1) {
        tree->insert(queries[i]);
    }

    auto end_time = std::chrono::system_clock::now();
    chrono::duration<double> elapsed_seconds = end_time - start_time;
    double total_time = elapsed_seconds.count();

    printTimes(queriesMeta, tree, total_time);
}

void lazy(vector <Tinterval> & queries, T leaf_size, T * queriesMeta) {
    auto start_time = std::chrono::system_clock::now();
    QMapLazy <Traits <T>> * qMap = new QMapLazy <Traits <T>>();
    Tree <Traits <T> > * tree = new Tree <Traits <T> >(leaf_size, qMap);

    for (int i = 0; i < queries.size(); i += 1) {
        tree->insert(queries[i]);
    }

    auto end_time = std::chrono::system_clock::now();
    chrono::duration<double> elapsed_seconds = end_time - start_time;
    double total_time = elapsed_seconds.count();

    printTimes(queriesMeta, tree, total_time);
}

int main(int argc, char** argv) {
    gflags::SetUsageMessage("Profiling");
    gflags::SetVersionString("1.0.0");
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    if (FLAGS_range_size > FLAGS_key_domain_size) {
        cout << "Range Size should be lower that domain" << endl;
        return 0;
    }

    srand (FLAGS_seed);
    vector <Tinterval> queries;
    T leaf_size = 0;

    if (FLAGS_distribution == "zipf") {
        queries = create_queries_zipf(FLAGS_queries, FLAGS_key_domain_size, FLAGS_range_size, FLAGS_random_range_size, FLAGS_percentage_point_queries);
    } else {
        queries = create_queries(FLAGS_queries, FLAGS_key_domain_size, FLAGS_range_size, FLAGS_random_range_size, FLAGS_percentage_point_queries);
    }

    T * queriesMeta = getQueriesMeta(queries);
    if (is_number(FLAGS_leaf_size)) {
        leaf_size = atol(FLAGS_leaf_size.c_str());
    } else if (FLAGS_leaf_size == "max_range") {
        leaf_size = queriesMeta[2];
    }

    if (FLAGS_strategy == "lazy") {
        lazy(queries, leaf_size, queriesMeta);
    } else if (FLAGS_strategy == "eager") {
        eager(queries, leaf_size, queriesMeta);
    } else if (FLAGS_strategy == "additional") {
        additional(queries, leaf_size, queriesMeta);
    }
}
