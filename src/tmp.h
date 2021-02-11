#ifndef JSDJASDJASJDJASD_H
#define JSDJASDJASJDJASD_H

#include "includes.h"
#include "interval.h"

using namespace std;

template <class T>
class Node {
public:
    typedef Node<T> Tnode;
    typedef Interval<T> Tinterval;

    Node * left;
    Node * right;
    Node * parent;
    Tinterval interval;

    vector <Tinterval *> * lazy;

    Node() {
        left = NULL;
        right = NULL;
        parent = NULL;
    }

    Node(const Tnode & other) : Node() {
        interval = Tinterval(other.interval);
        left = other.left;
        right = other.right;
        parent = other.parent;
    }

    Node(Tinterval interval) : Node() {
        this->interval = interval;
    }

    Node(Tinterval interval, Tinterval * query) : Node(interval) {
    }

    int maxDepth() {
        if (isLeaf()) {
            return 0;
        }
        int lDepth = 0;
        int rDepth = 0;
        if (left != NULL) {
            lDepth = left->maxDepth();
        }
        if (right != NULL) {
            rDepth = right->maxDepth();
        }

        if (lDepth > rDepth) {
            return(lDepth + 1);
        }
        return(rDepth + 1);
    }

    void updateLimits(bool recursive=true) {
        bool hasChange = false;
        if ((left != NULL) && left->interval.min < interval.min) {
            hasChange = true;
            interval.min = left->interval.min;
        }
        if ((right != NULL) && interval.max < right->interval.max) {
            hasChange = true;
            interval.max = right->interval.max;
        }
        if (parent != NULL && hasChange && recursive) {
            parent->updateLimits();
        }
    }

    bool isLeaf() {
        return (left == NULL && right == NULL);
    }

    void getLeafs(Tnode * visitor, vector<Tnode *> & leafs) {
        if(visitor != NULL) {
            if (visitor->isLeaf()) {
                leafs.push_back(visitor);
            }
            getLeafs(visitor->left, leafs);
            getLeafs(visitor->right, leafs);
        }
    }

    void getLeafs(vector<Tnode *> & leafs) {
        getLeafs(this, leafs);
    }

    void recursiveValidate(Tnode * node) {
        if (node->isLeaf()) {
            return;
        }
        if (node->left->interval.min != node->interval.min) {
            cout << "invalid left " << node->interval << endl;
            return;
        }
        if (node->right->interval.max != node->interval.max) {
            cout << "invalid right " << node->interval << endl;
            return;
        }
        recursiveValidate(node->left);
        recursiveValidate(node->right);
    }

    void recursiveValidate() {
        recursiveValidate(this);
    }
};




template <class T>
class LeafNode {

public:
    typedef Interval<T> Tinterval;
    typedef LeafNode<T> Tnode;
    Tnode * left;
    Tnode * right;
    Tnode * parent;
    T max;

    Tinterval interval;
    /*
        Intervals has to works as queries and range of leaf node
    */
    // The pair is composed by a pointer to the query and the current range
    // that belongs to the query.
    vector <pair<Tinterval *, Tinterval> > hashmap;

    LeafNode(Tinterval interval) {
        this->interval = interval;
        T size = interval.max - interval.min;
        left = NULL;
        right = NULL;
        parent = NULL;
    }

    void update_weights() {
        if (this->parent != NULL && this->max > parent->max) {
            parent->max = this->max;
            parent->update_weights();
        } else {
            max = interval.max;
        }
    }

    void printHashMap() {
        for (int i = 0; i < hashmap.size(); i += 1) {
            cout << *(hashmap[i].first) << "*" << hashmap[i].second << "*" << ", ";
        }
    }

    unsigned long long int checksum() {
        unsigned long long int value = 0;
        for (int i = 0; i < hashmap.size(); i += 1) {
            value += hashmap[i].second.checksum();
        }

        return value;
    }

    void print() {
        printf("[%d, %d](%d)", interval.min, interval.max, max);
    }
};


template <class Tr>
class LeafTree {

public:
    typedef typename Tr::T T;
    typedef typename Tr::Tinterval Tinterval;
    typedef LeafNode<T> Tnode;
    Tnode * root;

    LeafTree() {
        root = NULL;
    }

    void assign(Tinterval * query) {
        queue<Tinterval> pending;
        pending.push(*query);
        while (!pending.empty()) {
            Tnode ** visitor = &(this->root);
            Tinterval interval = pending.front();
            pending.pop();
            while ((*visitor) != NULL) {
                if (interval.min >= (*visitor)->interval.max) {
                    visitor = &((*visitor)->right);
                } else if (interval.max <= (*visitor)->interval.min) {
                    visitor = &((*visitor)->left);
                } else {
                    T left, right;
                    if (interval.min < (*visitor)->interval.min) {
                        left = (*visitor)->interval.min;
                        Tinterval tempInterval = Tinterval(interval.min, (*visitor)->interval.min);
                        pending.push(tempInterval);
                    } else {
                        left = interval.min;
                    }
                    if (interval.max > (*visitor)->interval.max) {
                        right = (*visitor)->interval.max;
                        Tinterval tempInterval = Tinterval((*visitor)->interval.max, interval.max);
                        pending.push(tempInterval);
                    } else {
                        right = interval.max;
                    }
                    (*visitor)->hashmap.push_back(make_pair(query, Tinterval(left, right)));
                    break;
                }
            }
        }
    }

    Tnode ** search(Tinterval interval, Tnode * & parent = NULL) {
        Tnode ** visitor = &(this->root);
        while ((*visitor) != NULL) {
            parent = *visitor;

            if (interval == (*visitor)->interval) {
                break;
            } else if (interval < (*visitor)->interval) {
                visitor = &((*visitor)->left);
            } else {
                visitor = &((*visitor)->right);
            }
        }

        return visitor;
    }

    bool insert(Tinterval & key) {
        Tnode * parent = NULL;
        Tnode ** searchNode = this->search(key, parent);
        if ((*searchNode) != NULL) {
            return false;
        }
        (*searchNode) = new Tnode(key);
        (*searchNode)->parent = parent;
        (*searchNode)->update_weights();

        return true;
    }

    void find(T key, vector <Tinterval *> & results, Tnode * visitor) {
        if (visitor == NULL) {
            return;
        }
        cout << "interval: " << visitor->interval << endl;
        if (visitor->right) {
            cout << "-->" << visitor->right->max << endl;
        }
        if (visitor->interval.intersects(key)) {
            results.push_back(&(visitor->interval));
        }
        if (visitor->left && key < visitor->left->max) {
            find(key, results, visitor->left);
        }
        if (visitor->right && key < visitor->right->max) {
            find(key, results, visitor->left);
        }
    }

    void find(T key, vector <Tinterval *> & results) {
        find(key, results, root);
    }

    void print() {
        print(this->root);
        printf("\n");
    }

    void print(Tnode * visitor) {
        if (visitor != NULL) {
            visitor->print();
            printf(" -> ");
            print(visitor->left);
            print(visitor->right);
        } else {
            printf("nil | ");
        }
    }

    void graphviz(Tnode *node, string & tree, string iter="") {
        if (node != NULL) {
            if (node->parent == NULL && node->left == NULL && node->right == NULL) {
                tree += node->interval.to_graphviz(iter);
                for (int i = 0; i < node->hashmap.size(); i += 1) {
                    tree += node->interval.to_graphviz(iter); + " -> " + node->hashmap[i].first->to_graphviz(iter) + " ";
                }
            }
            if (node->left != NULL) {
                tree += node->interval.to_graphviz(iter) + " -> " + node->left->interval.to_graphviz(iter) + " ";
            }
            if (node->right != NULL) {
                tree += node->interval.to_graphviz(iter) + " -> " + node->right->interval.to_graphviz(iter) + " ";
            }

            graphviz(node->left, tree, iter);
            graphviz(node->right, tree, iter);
        }
    }

    string graphviz(string iter=""){
        // string str = "digraph G {\n";
        string tree = "";
        graphviz(root, tree, iter);
        // str += tree + "}";
        return tree;
    }

    void get_nodes(Tnode * visitor, vector<Tnode *> & nodes) {
        if (visitor != NULL) {
            nodes.push_back(visitor);
            get_nodes(visitor->left, nodes);
            get_nodes(visitor->right, nodes);
        }
    }

    void getNodeCheckSum(Tnode * visitor, unsigned long long int & val) {
        if (visitor != NULL) {
            unsigned long long int chk = visitor->checksum();
            val += chk;
            getNodeCheckSum(visitor->left, val);
            getNodeCheckSum(visitor->right, val);
        }
    }

    unsigned long long int checksum() {
        unsigned long long int value = 0;
        getNodeCheckSum(root, value);

        return value;
    }

    void getLeafHash(Tnode * visitor) {
        if (visitor != NULL) {
            cout << "==>" << visitor->interval << " - ";
            visitor->printHashMap();
            cout << endl;
            getLeafHash(visitor->left);
            getLeafHash(visitor->right);
        }
    }

    void getLeafHash() {
        getLeafHash(root);
    }

    vector<Tnode *> nodes () {
        vector<Tnode *> result;
        get_nodes(root, result);

        return result;
    }

    long numIndexedQueries() {
        vector<Tnode *> nodes = this->nodes();
        long indexed = 0;

        for (int i = 0; i < nodes.size(); i += 1) {
            indexed += nodes[i]->hashmap.size();
        }

        return indexed;
    }

};


template <class Tr>
class QMapBase {
public:
    typedef typename Tr::Tinterval Tinterval;
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
    }

    virtual void _insert(Tnode * & node, Tinterval * interval) {}

    virtual void _transfer(Tnode * & from, Tnode * & to) {}

    virtual void _share(Tnode * & a, Tnode * & b) {}

    virtual void _merge(Tnode * & node) {}

    void insert(Tnode * & node, Tinterval * interval) {
        this->insertOps += 1;
        auto begin = std::chrono::system_clock::now();

        _insert(node, interval);

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

    virtual unsigned long long int checksum() {
        return 0;
    }

    virtual void summary() {}

    virtual string csv() {
        string s = to_string(this->numIndexedQueries()) + "," + to_string(this->insertOps) + "," + to_string(this->transferOps) + "," + to_string(this->shareOps) + "," + to_string(this->mergeOps) + ",";
        s += to_string(this->insertTime) + "," + to_string(this->transferTime) + "," + to_string(this->shareTime) + "," + to_string(this->mergeTime);

        return s;
    }

    virtual void postInsert() {
        // Used for LeafTree
    }

    virtual void printAllQueries() {}

    double elapsedTime() {
        return insertTime + mergeTime + transferTime + shareTime;
    }

    virtual long numIndexedQueries () {
        return 0;
    }
};


template <class Tr>
class QMapExtra: public QMapBase <Tr> {
public:
    typedef typename Tr::Tinterval Tinterval;
    typedef typename Tr::Tnode Tnode;

    void _insert(Tnode * & node, Tinterval * interval) {}

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

    void postInsert() {}
};


template <class Tr>
class QMapLazy : public QMapBase <Tr> {
    typedef typename Tr::Tinterval Tinterval;
    typedef typename Tr::Tnode Tnode;
    typedef vector <Tinterval *> qArray;
    typedef unordered_map<Tnode *, qArray> qMapType;

public:
    qMapType qMap;

    void _insert(Tnode * & node, Tinterval * interval) {
        qMap[node].emplace_back(interval);
    }

    void _transfer(Tnode * & from, Tnode * & to) {
        qMap[to] = qMap[from];
        qMap.erase(from);
    }

    void _share(Tnode * & a, Tnode * & b) {
        // Copy all the elements from A
        set<Tinterval *> tempSet;
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

        for (typename set<Tinterval *>::iterator it = tempSet.begin(); it != tempSet.end(); it++) {
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
                Tinterval intersection = it->first->interval.intersection(**jt);
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
    typedef typename Tr::Tnode Tnode;
    typedef pair<Tinterval *, Tinterval> qPair;
    typedef vector <qPair> qArray;
    typedef unordered_map<Tnode *, qArray> qMapType;

    struct classcomp {
        bool operator() (const qPair& lhs, const qPair& rhs) const
        {return lhs<rhs;}
    };

public:
    qMapType qMap;

    void updateIntersections (Tnode * & node) {
        for (typename vector <qPair>::iterator it = qMap[node].begin(); it != qMap[node].end();) {
            Tinterval intersection = it->first->intersection(node->interval);
            if (intersection.length()) {
                it->second = intersection;
                it++;
            } else {
                qMap[node].erase(it);
            }
        }
    }

    void _insert(Tnode * & node, Tinterval * interval) {
        Tinterval intersection = interval->intersection(node->interval);
        qMap[node].emplace_back(make_pair(interval, intersection));
    }

    void _transfer(Tnode * & from, Tnode * & to) {
        qMap[to] = qMap[from];
        this->updateIntersections(to);
        qMap.erase(from);
    }

    void _share(Tnode * & a, Tnode * & b) {
        // Copy all the elements from A
        set<qPair> tempSet;
        typename vector<qPair>::iterator it;
        qArray tempVectorA(qMap[a].begin(), qMap[a].end());

        for(size_t i = 0; i < qMap[b].size(); i++) {
            bool query_exists = false;
            for (size_t j = 0; j < tempVectorA.size(); j++) {
                if (qMap[b][i].first == tempVectorA[j].first) {
                    query_exists = true;
                    break;
                }
            }
            if (!query_exists) {
                tempVectorA.emplace_back(qMap[b][i]);
            }
        }
        qArray tempB(tempVectorA.begin(), tempVectorA.end());

        qMap.erase(a);
        qMap.erase(b);
        qMap[a] = tempVectorA;
        qMap[b] = tempB;

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
                val += jt->second.checksum();
            }
        }

        return val;
    }

    void printAllQueries() {
        for (typename qMapType::iterator it = qMap.begin(); it != qMap.end(); it++) {
            cout << it->first->interval << endl;
            for (typename qArray::iterator jt = it->second.begin(); jt != it->second.end(); jt++) {
                cout << "\t" << "( " << jt->first << " )" << jt->second << endl;
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



template <class C>
class Traits {
public:
    typedef C T;
    typedef Node<T> Tnode;
    typedef Interval<T> Tinterval;
};


template <class Tr>
class Tree {
public:
    typedef typename Tr::T T;
    typedef typename Tr::Tinterval Tinterval;
    typedef typename Tr::Tnode Tnode;

    Tnode * root;
    T M;

    QMapBase<Tr> * qMap;

    Tree(T M, QMapBase<Tr> * qMap) {
        root = NULL;
        this->M = M;
        this->qMap = qMap;
    }

    void search(Tinterval & interval, vector<Tinterval> & Q, Tnode * & node) {
        node = root;
        while(node != NULL && !node->isLeaf())  {
            Tinterval leftIntersection, rightIntersection;
            Tnode * sibling;
            bool isRight = false;

            if (node->left != NULL) {
                leftIntersection = interval.intersection(node->left->interval);
            }
            if (node->right != NULL) {
                rightIntersection = interval.intersection(node->right->interval);
            }
            // Its important to check first if the child nodes has intersections
            if (leftIntersection.length() > 0) {
                node = node->left;
                sibling = node->right;
            } else if (rightIntersection.length() > 0) {
                node = node->right;
                sibling = node->left;
                isRight = true;
            } else if (interval < node->interval) {
                node = node->left;
                sibling = node->right;
            } else {
                node = node->right;
                sibling = node->left;
                isRight = true;
            }
            if (isRight && leftIntersection.length() > 0) {
                Q.push_back(Tinterval(interval.min, leftIntersection.max));
                interval.min = leftIntersection.max;
            } else if (!isRight && rightIntersection.length() > 0) {
                Q.push_back(Tinterval(rightIntersection.min, interval.max));
                interval.max = rightIntersection.min;
            }
        }
    }

    void insert(Tinterval & interval) {
        vector <Tinterval> Q;
        get_intervals(interval, Q);
        bool controlInserts = Q.size() > 1;
        unordered_set<Tnode *> insertNodesTemp;
        typename unordered_set<Tnode *>::iterator itm;

        for (int i = 0; i < Q.size(); i += 1) {
            Tnode * S = NULL; // Points to the parent of N.
            Tinterval I(Q[i]);
            search(I, Q, S);
            if (S == NULL) {
                root = new Tnode(I);
                qMap->insert(root, &interval);
            } else {
                Tinterval J = I + S->interval;
                if (S->interval.min <= J.min && J.max <= S->interval.max) {
                    // Update new queries
                    // Todo: Check this if under different parameters
                    if (controlInserts) {
                        itm = insertNodesTemp.find(S);
                        if (itm == insertNodesTemp.end()) {
                            qMap->insert(S, &interval);
                            insertNodesTemp.insert(S);
                        }
                    } else {
                        qMap->insert(S, &interval);
                    }
                } else {
                    Tnode * T = new Tnode(*S);
                    Tnode * N = new Tnode(I);
                    S->interval = Tinterval(J);

                    qMap->transfer(S, T);
                    qMap->insert(N, &interval);

                    if (controlInserts) {
                        insertNodesTemp.insert(N);
                    }

                    if (T->interval < N->interval) {
                        S->left = T;
                        S->right = N;
                    } else {
                        S->left = N;
                        S->right = T;
                    }
                    T->parent = S;
                    N->parent = S;
                    update(S);

                    if (S->parent != NULL) {
                        S->parent->updateLimits();
                    }
                }
            }
        }
    }

    void update(Tnode * & node) {
        if (!node->isLeaf() && (node->left->interval.max == node->right->interval.min || node->right->interval.intersects(node->left->interval))) {
            if (node->interval.length() <= M) {
                update_merge(node);
            } else if(node->left->isLeaf() && node->right->isLeaf()) {
                udate_resize(node);
            }
        }
    }

    void update_merge(Tnode * & node) {
        if (node != NULL && node->interval.length() <= M && (node->left->interval.max == node->right->interval.min || node->right->interval.intersects(node->left->interval))) {
            if (node->left->interval.min < node->interval.min) {
                node->interval.min = node->left->interval.min;
            }
            if (node->right->interval.max > node->interval.max) {
                node->interval.max = node->right->interval.max;
            }
            qMap->merge(node);
            node->left = NULL;
            node->right = NULL;


            if (node->parent != NULL) {
                node->parent->updateLimits();
            }

            // Todo: Consider this recursive call
            update_merge(node->parent);
        }
    }

    void udate_resize(Tnode * & node) {
        T m = node->interval.midpoint();
        node->left->interval.max = m;
        node->right->interval.min = m;
        qMap->share(node->left, node->right);
    }

    string graphviz(string iter=""){
        // string str = "digraph G {\n";
        string tree = "";
        graphviz(root, tree, iter);
        // str += tree + "}";
        return tree;
    }

    T * getLeafsData() {
        vector<Tnode *> leafs;
        T inf = numeric_limits<T>::max();
        root->getLeafs(leafs);
        T size = leafs.size();
        T depth = root->maxDepth();
        T * res = new T[5];
        // Average node length
        res[0] = 0;
        // Min node length
        res[1] = inf;
        // Max node length
        res[2] = -inf;
        // Number of leaf nodes
        res[3] = size;
        // Max depth
        res[4] = depth;

        for (int i = 0; i < size; i += 1) {
            T width = leafs[i]->interval.length();
            res[0] += width;
            if (width < res[1]) {
                res[1] = width;
            }
            if (width > res[2]) {
                res[2] = width;
            }
        }

        res[0] = res[0] / size;

        return res;
    }

private:
    void get_intervals(const Tinterval interval, vector<Tinterval> & arr) {
        if (interval.length() <= M) {
            Tinterval tmp(interval.min, interval.max);
            arr.push_back(tmp);
        } else {
            T x = interval.min;
            while ((interval.max - x) > M) {
                Tinterval tmp (x, x + M);
                arr.push_back(tmp);
                x += M;
            }
            Tinterval tmp (x, interval.max);
            arr.push_back(tmp);
        }
    }

    void graphviz(Tnode *node, string & tree, string iter="") {
        if (node != NULL) {
            if (node->left == NULL && node->right == NULL) {
                tree += node->interval.to_graphviz(iter);
            }
            if (node->left != NULL) {
                tree += node->interval.to_graphviz(iter) + " -> " + node->left->interval.to_graphviz(iter) + " ";
            }
            if (node->right != NULL) {
                tree += node->interval.to_graphviz(iter) + " -> " + node->right->interval.to_graphviz(iter) + " ";
            }
            if (node->left == NULL && node->right == NULL) {
                tree += node->interval.to_graphviz(iter) + " [shape=box color=blue fillcolor=blue fontcolor=white style=filled]";
            }
            graphviz(node->left, tree, iter);
            graphviz(node->right, tree, iter);
        }
    }

};


#endif // JSDJASDJASJDJASD_H
