#include <gflags/gflags.h>

#include "../interval-base-tree/src/newtree.h"

#include "result.h"
#include "utils.h"

DEFINE_int64(queries, 100, "Number of queries");
DEFINE_int64(key_domain_size, 1000000, "Key domain size");
DEFINE_int64(leaf_size, 100000, "Leaf size");
DEFINE_int64(range_size, 100000, "Range of queries");
DEFINE_string(strategy, "raw", "Strategy");
DEFINE_string(distribution, "normal", "Random Distribution");
DEFINE_int64(iter, 0, "Define the iteration number");
DEFINE_int64(seed, 100, "Random Seed");


void printTimes(Tree <Traits <T> > * & tree, double total_time, double mapping_time = 0) {
    double tbt, mt, t2t, ttt;
    tbt = total_time - tree->qMap->elapsedTime();
    T * leafsData = tree->getLeafsData();

    cout << FLAGS_iter << ",";
    cout << FLAGS_strategy << "," << FLAGS_queries << "," << FLAGS_key_domain_size << ",";
    cout << FLAGS_leaf_size << "," << FLAGS_range_size << "," << FLAGS_distribution << ",";
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

void additional(vector <Tinterval> & queries) {
    auto start_time = std::chrono::system_clock::now();
    QMapExtra <Traits <T>> * qMap = new QMapExtra <Traits <T>>();
    Tree <Traits <T> > * tree = new Tree <Traits <T> >(FLAGS_leaf_size, qMap);

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

    printTimes(tree, total_time, mapping_time);
}

void eager(vector <Tinterval> & queries) {
    auto start_time = std::chrono::system_clock::now();
    QMapEager <Traits <T>> * qMap = new QMapEager <Traits <T>>();
    Tree <Traits <T> > * tree = new Tree <Traits <T> >(FLAGS_leaf_size, qMap);

    for (int i = 0; i < queries.size(); i += 1) {
        tree->insert(queries[i]);
    }

    auto end_time = std::chrono::system_clock::now();
    chrono::duration<double> elapsed_seconds = end_time - start_time;
    double total_time = elapsed_seconds.count();

    printTimes(tree, total_time);
}

void lazy(vector <Tinterval> & queries) {
    auto start_time = std::chrono::system_clock::now();
    QMapLazy <Traits <T>> * qMap = new QMapLazy <Traits <T>>();
    Tree <Traits <T> > * tree = new Tree <Traits <T> >(FLAGS_leaf_size, qMap);

    for (int i = 0; i < queries.size(); i += 1) {
        tree->insert(queries[i]);
    }

    auto end_time = std::chrono::system_clock::now();
    chrono::duration<double> elapsed_seconds = end_time - start_time;
    double total_time = elapsed_seconds.count();

    printTimes(tree, total_time);
}

int main(int argc, char** argv) {
    gflags::SetUsageMessage("Profiling");
    gflags::SetVersionString("1.0.0");
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    srand (FLAGS_seed);
    vector <Tinterval> queries;

    if (FLAGS_distribution == "zipf") {
        queries = create_queries_zipf(FLAGS_queries, FLAGS_key_domain_size, FLAGS_range_size);
    } else {
        queries = create_queries(FLAGS_queries, FLAGS_key_domain_size, FLAGS_range_size);
    }

    if (FLAGS_strategy == "lazy") {
        lazy(queries);
    } else if (FLAGS_strategy == "eager") {
        eager(queries);
    } else if (FLAGS_strategy == "additional") {
        additional(queries);
    }
}
