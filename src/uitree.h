#ifndef UITREE_H
#define UITREE_H

#include "includes.h"
#include "interval.h"
#include "query.h"
#include "qmap.h"

template <class T>
vector<Interval<T> > split_queries_in_intervals(Query<T> * query, T M) {
    vector<Interval<T> > arr;
    if (query->interval.length() <= M) {
        Interval<T> tmp(query->interval.min, query->interval.max);
        arr.push_back(tmp);
    } else {
        T x = query->interval.min;
        while ((query->interval.max - x) > M) {
            Interval<T> tmp(x, x + M);
            arr.push_back(tmp);
            x += M;
        }
        Interval<T> tmp(x, query->interval.max);
        arr.push_back(tmp);
    }

    return arr;
}

template <class T>
class UINode {
public:
    typedef UINode<T> Tnode;
    typedef Interval<T> Tinterval;

    Tnode * left;
    Tnode * right;
    Tnode * parent;
    Tinterval interval;

    vector <Tinterval *> * lazy;

    UINode() {
        left = NULL;
        right = NULL;
        parent = NULL;
    }

    UINode(const Tnode & other) : UINode() {
        interval = Tinterval(other.interval);
        left = other.left;
        right = other.right;
        parent = other.parent;
    }

    UINode(Tinterval interval) : UINode() {
        this->interval = interval;
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



template <class C>
class Traits {
public:
    typedef C T;
    typedef UINode<T> Tnode;
    typedef Interval<T> Tinterval;
    typedef Query<T> Tquery;
};


template <class Tr>
class UITree {
public:
    typedef typename Tr::T T;
    typedef typename Tr::Tinterval Tinterval;
    typedef typename Tr::Tnode Tnode;
    typedef Query<T> Tquery;

    Tnode * root;
    T M;

    QMapBase<Tr> * qMap;

    UITree(T M, QMapBase<Tr> * qMap) {
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

    void insert(Tquery * query) {
        vector<Tinterval> Q = split_queries_in_intervals(query, M);
        bool controlInserts = Q.size() > 1;
        unordered_set<Tnode *> insertNodesTemp;
        typename unordered_set<Tnode *>::iterator itm;

        for (int i = 0; i < Q.size(); i++){
            Tnode * S = NULL; // Points to the parent of N.
            Tinterval I(Q[i]);
            search(I, Q, S);
            if (S == NULL) {
                root = new Tnode(I);
                qMap->insert(root, query);
            } else {
                Tinterval J = I + S->interval;
                if (S->interval.min <= J.min && J.max <= S->interval.max) {
                    // Update new queries
                    // Todo: Check this if under different parameters
                    if (controlInserts) {
                        itm = insertNodesTemp.find(S);
                        if (itm == insertNodesTemp.end()) {
                            qMap->insert(S, query);
                            insertNodesTemp.insert(S);
                        }
                    } else {
                        qMap->insert(S, query);
                    }
                } else {
                    Tnode * T = new Tnode(*S);
                    Tnode * N = new Tnode(I);
                    S->interval = Tinterval(J);

                    qMap->transfer(S, T);
                    qMap->insert(N, query);

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

#endif // UITREE_H
