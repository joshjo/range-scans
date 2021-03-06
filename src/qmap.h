#ifndef QMAP_H
#define QMAP_H

#include "includes.h"
#include "interval.h"
#include "query.h"
#include "ltree.h"

template <class Tr>
class QMapBase {
public:
    typedef typename Tr::Tinterval Tinterval;
    typedef typename Tr::Tquery Tquery;
    typedef typename Tr::Tnode Tnode;

    int insertOps;
    int mergeOps;
    int transferOps;
    int shareOps;
    int maxSizeSet;
    int indexed;

    double insertTime;
    double mergeTime;
    double transferTime;
    double shareTime;
    double postInsertTime;

    QMapBase() {

        indexed = 0;
        mergeOps = 0;
        shareOps = 0;
        transferOps = 0;
        insertOps = 0;
        maxSizeSet = 0;

        insertTime = 0;
        mergeTime = 0;
        transferTime = 0;
        shareTime = 0;
        postInsertTime = 0;
    }

    virtual void _insert(Tnode * & node, Tquery * query) {}

    virtual void _transfer(Tnode * & from, Tnode * & to) {}

    virtual void _share(Tnode * & a, Tnode * & b) {}

    virtual void _merge(Tnode * & node) {}


    virtual void _postInsert(vector<Tquery *> & queries, vector<Tnode *> & leafs) {
        /*
        This function is used for additional-tree data structure
        The Lazy and Eager won't execute anything.
        */
    }

    void insert(Tnode * & node, Tquery * query) {
        this->insertOps += 1;
        auto begin = std::chrono::system_clock::now();

        _insert(node, query);

        auto end = std::chrono::system_clock::now();
        chrono::duration<double> elapsed_seconds = end - begin;
        this->insertTime += elapsed_seconds.count();
    }

    void transfer(Tnode * & from, Tnode * & to) {
        this->transferOps += 1;
        auto begin = std::chrono::system_clock::now();

        _transfer(from, to);

        auto end = std::chrono::system_clock::now();
        chrono::duration<double> elapsed_seconds = end - begin;
        this->transferTime += elapsed_seconds.count();
    }

    void share(Tnode * & a, Tnode * & b) {
        this->shareOps += 1;
        auto begin = std::chrono::system_clock::now();

        _share(a, b);

        auto end = std::chrono::system_clock::now();
        chrono::duration<double> elapsed_seconds = end - begin;
        this->shareTime += elapsed_seconds.count();
    }

    void merge(Tnode * & node) {
        this->mergeOps += 1;
        auto begin = std::chrono::system_clock::now();

        _merge(node);

        auto end = std::chrono::system_clock::now();
        chrono::duration<double> elapsed_seconds = end - begin;
        this->mergeTime += elapsed_seconds.count();
    }

    void postInsert(vector<Tquery *> & queries, vector<Tnode *> & leafs) {
        auto begin = std::chrono::system_clock::now();

        _postInsert(queries, leafs);

        auto end = std::chrono::system_clock::now();
        chrono::duration<double> elapsed_seconds = end - begin;
        this->postInsertTime += elapsed_seconds.count();
    }

    virtual unsigned long long int checksum() {
        return 0;
    }

    virtual void summary() {}

    virtual string csv() {
        string s = to_string(this->numIndexedQueries()) + "," + to_string(this->insertOps) + "," + to_string(this->transferOps) + "," + to_string(this->shareOps) + "," + to_string(this->mergeOps) + ",";
        s += to_string(this->insertTime) + "," + to_string(this->transferTime) + "," + to_string(this->shareTime) + "," + to_string(this->mergeTime);

        return s;
    }

    virtual unordered_map <Tquery *, vector<Tquery *> > plain() {
        unordered_map <Tquery *, vector<Tquery *> > qm;
        return qm;
    }

    virtual void print () {}

    virtual void printAllQueries() {}

    double elapsedTime() {
        return insertTime + mergeTime + transferTime + shareTime;
    }

    virtual long numIndexedQueries () {
        return 0;
    }
};


template <class Tr>
class QMapAdditional: public QMapBase <Tr> {
public:
    typedef typename Tr::Tinterval Tinterval;
    typedef typename Tr::Tquery Tquery;
    typedef typename Tr::Tnode Tnode;
    typedef typename Tr::T T;
    typedef vector <Tquery *> qArray;
    typedef unordered_map<Tnode *, qArray> qMapType;

    LeafTree<Tr> ltree;

    void _insert(Tnode * & node, Tquery * query) {}

    void _transfer(Tnode * & from, Tnode * & to) {}

    void _share(Tnode * & a, Tnode * & b) {}

    void _merge(Tnode * & node) {}

    unsigned long long int checksum() {
        return 0;
    }

    void summary() {
        cout << "indexed      : " << this->indexed << endl;
        cout << "insert ops   : " << 0 << endl;
        cout << "transfer ops : " << 0 << endl;
        cout << "share ops    : " << 0 << endl;
        cout << "merge ops    : " << 0 << endl;

        cout << "insert time  : " << 0 << endl;
        cout << "transfer time: " << 0 << endl;
        cout << "share time   : " << 0 << endl;
        cout << "merge time   : " << 0 << endl;
    }

    string csv() {
        string s = to_string(this->indexed) + ",0,0,0,0,";
        s += "0,0,0,0";

        return s;
    }

    unordered_map<Tquery *, qArray> plain() {
        unordered_map<Tquery *, vector<Tquery *> > qm;
        vector <LeafNode<T> *> nodes = ltree.nodes();

        for (size_t i = 0; i < nodes.size(); i++) {
            Tquery * query = new Tquery(nodes[i]->interval);
            qm[query] = nodes[i]->queries;
        }

        return qm;
    }

    void _postInsert(vector<Tquery *> & queries, vector<Tnode *> & leafs) {
        random_shuffle(leafs.begin(), leafs.end());
        for (size_t i = 0; i < leafs.size(); i++) {
            ltree.insert(leafs[i]->interval);
        }
        for (size_t i = 0; i < queries.size(); i++) {
            ltree.assign(queries[i]);
        }
    }
};


template <class Tr>
class QMapLazy : public QMapBase <Tr> {
    typedef typename Tr::Tinterval Tinterval;
    typedef typename Tr::Tquery Tquery;
    typedef typename Tr::Tnode Tnode;
    typedef vector <Tquery *> qArray;
    typedef unordered_map<Tnode *, qArray> qMapType;

public:
    qMapType qMap;

    void _insert(Tnode * & node, Tquery * query) {
        qMap[node].emplace_back(query);
    }

    void _transfer(Tnode * & from, Tnode * & to) {
        qMap[to] = qMap[from];
        qMap.erase(from);
    }

    void _share(Tnode * & a, Tnode * & b) {
        // Copy all the elements from A
        set<Tquery *> tempSet;
        typename qArray::iterator it;
        // Copy all the elements from B
        for (it = qMap[a].begin(); it != qMap[a].end(); it++) {
            tempSet.insert(*it);
        }
        for (it = qMap[b].begin(); it != qMap[b].end(); it++) {
            tempSet.insert(*it);
        }

        qArray tempA;
        tempA.reserve(tempSet.size());

        for (typename set<Tquery *>::iterator it = tempSet.begin(); it != tempSet.end(); it++) {
            tempA.emplace_back(*it);
        }

        qArray tempB(tempA.begin(), tempA.end());

        qMap.erase(a);
        qMap.erase(b);
        qMap[a] = tempA;
        qMap[b] = tempB;

    }

    void _merge(Tnode * & node) {
        qArray temp;

        Tnode * a = node->left;
        Tnode * b = node->right;

        vector<Tnode *> leafs;

        if (a != NULL) {
            a->getLeafs(leafs);
        }
        if (b != NULL) {
            b->getLeafs(leafs);
        }

        for (size_t i = 0; i < leafs.size(); i+= 1) {
            Tnode * n = leafs[i];
            for (typename qArray::iterator it = qMap[n].begin(); it != qMap[n].end(); it++) {
                temp.push_back((*it));
            }
            qMap.erase(n);
        }

        if (temp.size() > 0) {
            qMap[node] = temp;
        }
    }

    unsigned long long int checksum() {
        unsigned long long int val = 0;
        for (typename qMapType::iterator it = qMap.begin(); it != qMap.end(); it++) {
            for (typename qArray::iterator jt = it->second.begin(); jt != it->second.end(); jt++) {
                Tinterval intersection = it->first->interval.intersection((*jt)->interval);
                val += intersection.checksum();
            }
        }

        return val;
    }

    long numIndexedQueries () {
        long indexed = 0;
        for (typename qMapType::iterator it = qMap.begin(); it != qMap.end(); it++) {
            indexed += it->second.size();
        }
        return indexed;
    }

    void print () {
        for (typename qMapType::iterator itq = qMap.begin(); itq != qMap.end(); itq++) {
            cout << "node: " << itq->first->interval << endl;

            for (size_t i = 0; i < itq->second.size(); i++) {
                cout << "\t" << "q: " << itq->second[i]->interval << endl;
            }
            cout << endl;
        }
    }

    unordered_map<Tquery *, qArray> plain() {
        unordered_map<Tquery *, vector<Tquery *> > qm;
        for (typename qMapType::iterator it = qMap.begin(); it != qMap.end(); it++) {
            Tquery * query = new Tquery(it->first->interval);
            qm[query] = it->second;
        }

        return qm;
    }

    void summary() {
        cout << "size: " << qMap.size() << endl;

        cout << "indexed      : " << this->numIndexedQueries() << endl;
        cout << "insert ops   : " << this->insertOps << endl;
        cout << "transfer ops : " << this->transferOps << endl;
        cout << "share ops    : " << this->shareOps << endl;
        cout << "merge ops    : " << this->mergeOps << endl;

        cout << "insert time  : " << this->insertTime << endl;
        cout << "transfer time: " << this->transferTime << endl;
        cout << "share time   : " << this->shareTime << endl;
        cout << "merge time   : " << this->mergeTime << endl;
    }

    void postInsert() {}
};


template <class Tr>
class QMapEager : public QMapBase <Tr> {

    typedef typename Tr::Tinterval Tinterval;
    typedef typename Tr::Tquery Tquery;
    typedef typename Tr::Tnode Tnode;
    typedef vector <Tquery *> qArray;
    typedef unordered_map<Tnode *, qArray> qMapType;

    // struct classcomp {
    //     bool operator() (const qPair& lhs, const qPair& rhs) const
    //     {return lhs<rhs;}
    // };

public:
    qMapType qMap;

    void updateIntersections (Tnode * & node) {
        for (typename qArray::iterator it = qMap[node].begin(); it != qMap[node].end();) {
            Tinterval intersection = (*it)->interval.intersection(node->interval);
            if (intersection.length()) {
                it++;
            } else {
                qMap[node].erase(it);
            }
        }
    }

    void _insert(Tnode * & node, Tquery * query) {
        qMap[node].emplace_back(query);
    }

    void _transfer(Tnode * & from, Tnode * & to) {
        qMap[to] = qMap[from];
        this->updateIntersections(to);
        qMap.erase(from);
    }

    void _share(Tnode * & a, Tnode * & b) {
        // Copy all the elements from A
        typename qArray::iterator it;
        qArray tempVectorA(qMap[a].begin(), qMap[a].end());

        for(size_t i = 0; i < qMap[b].size(); i++) {
            bool query_exists = false;
            for (size_t j = 0; j < tempVectorA.size(); j++) {
                // Todo: check if we can use ids here
                if (qMap[b][i] == tempVectorA[j]) {
                    query_exists = true;
                    break;
                }
            }
            if (!query_exists) {
                tempVectorA.emplace_back(qMap[b][i]);
            }
        }
        qArray tempVectorB(tempVectorA.begin(), tempVectorA.end());

        qMap.erase(a);
        qMap.erase(b);
        qMap[a] = tempVectorA;
        qMap[b] = tempVectorB;

        updateIntersections(a);
        updateIntersections(b);
    }

    void _merge(Tnode * & node) {
        qArray temp;

        Tnode * a = node->left;
        Tnode * b = node->right;

        vector<Tnode *> leafs;

        if (a != NULL) {
            a->getLeafs(leafs);
        }
        if (b != NULL) {
            b->getLeafs(leafs);
        }

        for (size_t i = 0; i < leafs.size(); i+= 1) {
            Tnode * n = leafs[i];
            for (typename qArray::iterator it = qMap[n].begin(); it != qMap[n].end(); it++) {
                temp.push_back((*it));
            }
            qMap.erase(n);
        }

        if (temp.size() > 0) {
            qMap[node] = temp;
            updateIntersections(node);
        }
    }


    unsigned long long int checksum() {
        unsigned long long int val = 0;
        for (typename qMapType::iterator it = qMap.begin(); it != qMap.end(); it++) {
            for (typename qArray::iterator jt = it->second.begin(); jt != it->second.end(); jt++) {
                Tinterval intersection = it->first->interval.intersection((*jt)->interval);
                val += intersection.checksum();
            }
        }

        return val;
    }

    void printAllQueries() {
        for (typename qMapType::iterator it = qMap.begin(); it != qMap.end(); it++) {
            cout << it->first->interval << endl;
            for (typename qArray::iterator jt = it->second.begin(); jt != it->second.end(); jt++) {
                cout << "\t" << "( " << (*jt)->interval << " )" << endl;
            }
        }
    }

    long numIndexedQueries () {
        long indexed = 0;
        for (typename qMapType::iterator it = qMap.begin(); it != qMap.end(); it++) {
            indexed += it->second.size();
        }

        return indexed;
    }

    void summary() {
        cout << "size: " << qMap.size() << endl;

        cout << "indexed      : " << this->numIndexedQueries() << endl;
        cout << "insert ops   : " << this->insertOps << endl;
        cout << "transfer ops : " << this->transferOps << endl;
        cout << "share ops    : " << this->shareOps << endl;
        cout << "merge ops    : " << this->mergeOps << endl;

        cout << "insert time  : " << this->insertTime << endl;
        cout << "transfer time: " << this->transferTime << endl;
        cout << "share time   : " << this->shareTime << endl;
        cout << "merge time   : " << this->mergeTime << endl;
    }

    void postInsert() {}
};

#endif // QMAP_H
